#include <QApplication>
#include <QSurfaceFormat>
#include <QVTKOpenGLNativeWidget.h>

#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "MainWindow.hpp"

int main(int argc, char* argv[]) {
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


    spdlog::info("Application start.");

    QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    const int ret = a.exec();

    spdlog::info("Application quit.");
    return ret;
}
