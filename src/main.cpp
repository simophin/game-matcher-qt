#include <QApplication>

#include "MainWindow.h"
#include "models.h"
#include "ClubRepository.h"

#include "GameMatcher.h"
#include "CollectionUtils.h"
#include "span.h"


static void printAllocations(const QString &prefix, nonstd::span<GameAllocation> allocations, const QHash<MemberId, Member> &members) {
    std::sort(allocations.begin(), allocations.end(), [](const GameAllocation &a, const GameAllocation &b) {
        if (a.courtId == b.courtId) return a.memberId < b.memberId;
        return a.courtId < b.courtId;
    });

    QString out = prefix;
    out += QStringLiteral(": ");
    auto iter = allocations.begin();
    while (iter != allocations.end()) {
        out += QStringLiteral("[");
        for (int i = 0; i < 4; i++) {
            auto &m = members[(iter++)->memberId];
            out += QObject::tr("%1(%2,%3)").arg(QString::number(m.id), QString::number(m.level), m.genderString().left(1).toUpper());
            if (i < 3) out += QStringLiteral(", ");
        }
        out += QStringLiteral("], ");
    }

    qDebug() << out;
}


static void testMatcher() {
    ClubRepository repo;
    if (!repo.open(QStringLiteral("/Users/fanchao/Temp/badmintonclub"))) {
        throw "Unable to open repo";
    }

    if (auto sessionId = repo.getLastSession()) {
        auto members = repo.getMembers(CheckedIn{*sessionId});
        while (members.size() > 20) {
            members.pop_back();
        }

        auto session = repo.getSession(*sessionId);
        QVector<CourtId> courts;
        for (const auto &court : session->courts) {
            courts.append(court.id);
        }

        auto memberById = associateBy<QHash<MemberId, Member>>(members, [](auto &m) { return m.id; });

        std::vector<GameAllocation> allocations;

        auto result = GameMatcher::match(allocations, members, courts, 4, 0);
        for (auto &game : result) game.gameId = 0;
        printAllocations(QStringLiteral("Game 1"), result, memberById);

        allocations.insert(allocations.end(), result.begin(), result.end());
        result = GameMatcher::match(allocations, members, courts, 4, 1);
        for (auto &game : result) game.gameId = 1;
        printAllocations(QStringLiteral("Game 2"), result, memberById);

        allocations.insert(allocations.end(), result.begin(), result.end());
        result = GameMatcher::match(allocations, members, courts, 4, 2);
        for (auto &game : result) game.gameId = 2;
        printAllocations(QStringLiteral("Game 3"), result, memberById);

        allocations.insert(allocations.end(), result.begin(), result.end());
        result = GameMatcher::match(allocations, members, courts, 4, 3);
        for (auto &game : result) game.gameId = 3;
        printAllocations(QStringLiteral("Game 4"), result, memberById);
    }
}

int main(int argc, char **argv) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    qRegisterMetaType<GameId>("GameId");
    qRegisterMetaType<CourtId>("CourtId");
    qRegisterMetaType<SessionId>("SessionId");
    qRegisterMetaType<MemberId>("MemberId");
    qRegisterMetaType<SettingKey>("SettingKey");
    qRegisterMetaType<ClubInfo>();
    qRegisterMetaType<CourtPlayers>();

    QApplication app(argc, argv);

    QCoreApplication::setOrganizationName(QStringLiteral("Cloudwalker"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("fanchao.nz"));
    QCoreApplication::setApplicationName(QStringLiteral("Game Matcher"));


    testMatcher();

//    MainWindow main(nullptr);
//    main.show();

    return app.exec();
}