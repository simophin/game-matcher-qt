//
// Created by Fanchao Liu on 9/08/20.
//

#define CATCH_CONFIG_RUNNER

#include <catch2/catch.hpp>
#include <QApplication>
#include "models.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    registerModels();

    return Catch::Session().run( argc, argv );
}