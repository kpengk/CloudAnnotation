#include "MainWindow.hpp"

#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

#include <cxxopts/cxxopts.hpp>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <Dbghelp.h>
#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QTranslator>
#include <Windows.h>
#include <iostream>
#include <stdio.h>
#include <time.h>

#pragma comment(lib, "Dbghelp.lib")

LONG __stdcall crush_callback(EXCEPTION_POINTERS* ep) {
    time_t t = time(NULL) + 8 * 3600;
    tm* p = gmtime(&t);

    char fname[MAX_PATH] = {0};
    snprintf(fname, MAX_PATH, "dump_%d-%d-%d_%d_%d_%d.DMP", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
             (p->tm_hour) % 24, p->tm_min, p->tm_sec);

    HANDLE hFile = CreateFileA(fname, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return EXCEPTION_CONTINUE_SEARCH;
    }

    MINIDUMP_EXCEPTION_INFORMATION exceptioninfo;
    exceptioninfo.ExceptionPointers = ep;
    exceptioninfo.ThreadId = GetCurrentThreadId();
    exceptioninfo.ClientPointers = FALSE;

    if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithFullMemory, &exceptioninfo,
                           NULL, NULL)) {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    CloseHandle(hFile);
    return EXCEPTION_EXECUTE_HANDLER;
}

void initQss() {
    QFile file("config/skin.qss");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(nullptr, "File not found",
                              QString("%1/config/skin.qss").arg(QCoreApplication::applicationDirPath()));
    }

    QTextStream in(&file);
    QString css = in.readAll();
    qApp->setStyleSheet(css);

    return;
}

int main(int argc, char* argv[]) {
    cxxopts::Options options("CloudAnnotation", "3D point cloud annotation tool.");
    options.add_options()("c,console", "Enable console", cxxopts::value<bool>()->default_value("false"))(
        "d,coredump", "Enable core dump", cxxopts::value<bool>()->default_value("false"))("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
#ifdef _WIN32
        AllocConsole();
        AttachConsole(ATTACH_PARENT_PROCESS);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
#endif
        std::cout << options.help() << std::endl;
        system("pause");
#ifdef _WIN32
        FreeConsole();
#endif
        exit(0);
    }
    if (result["console"].as<bool>()) {
#ifdef _WIN32
        AllocConsole();
        AttachConsole(ATTACH_PARENT_PROCESS);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);
#endif
    }
    if (result["coredump"].as<bool>()) {
        SetUnhandledExceptionFilter(crush_callback);
    }

    // Create a file rotating logger with 20MB size max and 10 rotated files
    constexpr auto maxSize{1048576 * 20};
    constexpr auto maxFiles{10};
    std::vector<spdlog::sink_ptr> sinks;
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
    sinks.push_back(
        std::make_shared<spdlog::sinks::rotating_file_sink_mt>("logs/cloud_annotation.log", maxSize, maxFiles));
    auto logger = std::make_shared<spdlog::logger>("new_default_logger", std::begin(sinks), std::end(sinks));
    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::warn);
    spdlog::set_default_logger(logger);
    spdlog::flush_every(std::chrono::seconds(5));
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%L]%$ [tid %t] %v");

    QCoreApplication::setApplicationName("Cloud Aannotation");
    QCoreApplication::setOrganizationName("example");
    QCoreApplication::setOrganizationDomain("example.com");

    spdlog::info("Application start.");

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication a(argc, argv);

    initQss();

    QTranslator trans;
    const bool ok = trans.load("./translations/cloud_annotation_" + QLocale().name());
    Q_UNUSED(ok);
    QCoreApplication::installTranslator(&trans);

    MainWindow w;
    w.show();
    int exitCode{};
    // let's rock!
    try {
        exitCode = a.exec();
    } catch (const std::exception& e) {
        QMessageBox::warning(nullptr, "Cloud annotation crashed!",
                             QString("Hum, it seems that cloud annotation has crashed... Sorry about that :)\n") +
                                 QString::fromLocal8Bit(e.what()));
    } catch (...) {
        QMessageBox::warning(nullptr, "Cloud annotation crashed!",
                             "Hum, it seems that cloud annotation has crashed... Sorry about that :)");
    }

    spdlog::info("Application quit.");
    spdlog::shutdown();
    if (result["console"].as<bool>()) {
#ifdef Q_OS_WIN
        FreeConsole();
#endif
    }

    return exitCode;
}
