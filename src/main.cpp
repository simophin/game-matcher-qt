#include <QApplication>
#include <QFontDatabase>
#include <QStyleFactory>

#include "models.h"
#include "ClubRepository.h"

#include "GameMatcher.h"
#include "CollectionUtils.h"
#include "span.h"
#include "MainWindow.h"


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
    std::unique_ptr<ClubRepository> repo(ClubRepository::open(nullptr, QStringLiteral("/Users/fanchao/Temp/badmnton.clubfile")));
    if (!repo) {
        throw "Unable to open repo";
    }

    if (auto sessionId = repo->getLastSession()) {
        auto members = repo->getMembers(CheckedIn{*sessionId});
        while (members.size() > 50) {
            members.pop_back();
        }

        auto session = repo->getSession(*sessionId);
        QVector<CourtId> courts;
        for (const auto &court : session->courts) {
            courts.append(court.id);
        }
//        while (courts.size() > 2) {
//            courts.pop_back();
//        }

        auto memberById = associateBy<QHash<MemberId, Member>>(members, [](auto &m) { return m.id; });

        std::vector<GameAllocation> allocations;
        auto [levelMin, levelMax] = repo->getLevelRange();

        const auto playerPerCourt = 4;

        auto result = GameMatcher::match(allocations, members, courts, playerPerCourt, levelMin, levelMax, 0);
        for (auto &game : result) game.gameId = 0;
        printAllocations(QStringLiteral("Game 1"), result, memberById);

        allocations.insert(allocations.end(), result.begin(), result.end());
        result = GameMatcher::match(allocations, members, courts, playerPerCourt, levelMin, levelMax, 1);
        for (auto &game : result) game.gameId = 1;
        printAllocations(QStringLiteral("Game 2"), result, memberById);

        allocations.insert(allocations.end(), result.begin(), result.end());
        result = GameMatcher::match(allocations, members, courts, playerPerCourt, levelMin, levelMax, 2);
        for (auto &game : result) game.gameId = 2;
        printAllocations(QStringLiteral("Game 3"), result, memberById);

        allocations.insert(allocations.end(), result.begin(), result.end());
        result = GameMatcher::match(allocations, members, courts, playerPerCourt, levelMin, levelMax, 3);
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
    qRegisterMetaType<CourtPlayers>();
    qRegisterMetaType<Member>();

    QApplication app(argc, argv);
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoMono-Regular.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSans-Bold.ttf"));
    QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSansMono-Medium.ttf"));
    auto fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/NotoSans-Regular.ttf"));
    QFont font(QFontDatabase::applicationFontFamilies(fontId).first(), 16);
    QApplication::setFont(font);


    QCoreApplication::setOrganizationName(QStringLiteral("Cloudwalker"));
    QCoreApplication::setOrganizationDomain(QStringLiteral("fanchao.nz"));
    QCoreApplication::setApplicationName(QStringLiteral("Game Matcher"));

    MainWindow mainWindow;
    mainWindow.showMaximized();

//    testMatcher();

    return app.exec();
}