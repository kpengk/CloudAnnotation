#include "AboutDialog.hpp"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(const QString& version, QWidget* parent)
    : QDialog{parent}
    , ui{new Ui::AboutDialog}
    , version_str_{version} {
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    QString compilation_info;
    compilation_info = versionLongStr(true);
    compilation_info += QStringLiteral("<br><i>Compiled with");

#if defined(_MSC_VER)
    compilation_info += QStringLiteral(" MSVC %1 and").arg(_MSC_VER);
#endif

    compilation_info += QStringLiteral(" Qt %1").arg(QT_VERSION_STR);
    compilation_info += QStringLiteral("</i>");

    QString html_text = ui->labelText->text();
    const QString enriched_html_text = html_text.arg(compilation_info);

    ui->labelText->setText(enriched_html_text);
}

AboutDialog::~AboutDialog() { delete ui; }

QString AboutDialog::versionLongStr(bool includeOS) const {
    QString ver_str = version_str_;
    const QString arch = QStringLiteral("%1-bit").arg(sizeof(void*) * 8);

    if (includeOS) {
#if defined(Q_OS_WIN)
        const QString platform("Windows");
#elif defined(Q_OS_MAC)
        const QString platform("macOS");
#elif defined(Q_OS_LINUX)
        const QString platform("Linux");
#else
        const QString platform("Unknown OS");
#endif
        ver_str += QStringLiteral(" [%1 %2]").arg(platform, arch);
    } else {
        ver_str += QStringLiteral(" [%1]").arg(arch);
    }

#ifdef QT_DEBUG
    ver_str += QStringLiteral(" [DEBUG]");
#endif

    return ver_str;
}
