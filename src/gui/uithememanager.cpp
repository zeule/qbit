/*
 * Bittorrent Client using Qt and libtorrent.
 * Copyright (C) 2019  Prince Gupta <jagannatharjun11@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 *
 * In addition, as a special exception, the copyright holders give permission to
 * link this program with the OpenSSL project's "OpenSSL" library (or with
 * modified versions of it that use the same license as the "OpenSSL" library),
 * and distribute the linked executables. You must obey the GNU General Public
 * License in all respects for all of the code used other than "OpenSSL".  If
 * you modify file(s), you may extend this exception to your version of the
 * file(s), but you are not obligated to do so. If you do not wish to do so,
 * delete this exception statement from your version.
 */

#include "uithememanager.h"

#include "config.h"

#include <QApplication>
#include <QFile>
#include <QIcon>
#include <QPainter>
#include <QPalette>
#include <QResource>
#include <QStandardPaths>

#include "base/bittorrent/torrenthandle.h"
#include "base/iconprovider.h"
#include "base/logger.h"
#include "base/preferences.h"
#include "base/settingvalue.h"
#include "base/utils/fs.h"

#include "theme/colortheme.h"
#include "theme/themeprovider.h"

#ifdef HAVE_KF5ICONTHEMES
#include <KIconLoader>
#endif

#include <fstream>
#include <regex>
#include <stdexcept>
#include <sstream>

namespace
{
    const QLatin1String speedometerIconName {"speedometer"};
    const QLatin1String speedometerFlippedIconName {"speedometer-flipped"};
}

class UIThemeManager::SVGManipulator
{
public:
    SVGManipulator();

    void replaceSVGFillColor(const QString& svgFile, const QString &newSvgFile, const QColor& newColor);
private:
    // !! Both regular expressions contain the same number of caption groups !!
    // matches "color:xxxxx"
    std::regex m_cssColorRegex;
    // matches "fill="color" or "fill:color;"
    std::regex m_fillColorRegex;
};

UIThemeManager::SVGManipulator::SVGManipulator()
    : m_cssColorRegex(R"regex((color)(:"?)[^;|^\s]+(;"?))regex")
    , m_fillColorRegex(R"regex(( fill)(:|=")(?:(?!none|;|").)+(;|"))regex")
{
}

void UIThemeManager::SVGManipulator::replaceSVGFillColor(const QString& svgFile, const QString &newSvgFile,
                                                          const QColor& newColor)
{
    // TODO detect and upack zipped SVGs
    QFile inp(svgFile);
    if (!inp.open(QIODevice::ReadOnly))
        throw std::runtime_error("Could not read input file");

    std::ofstream res(newSvgFile.toStdString());

    if (res.fail())
        throw std::runtime_error("Could not create result file");

    Q_ASSERT(inp.size() < 100 * 1024); // SVG icons are small files

    std::stringstream svgContentsStream;
    svgContentsStream << inp.readAll().constData();
    std::string svgContents = svgContentsStream.str();

    // we support two types of colored icons:
    // 1) the file defines CSS classes, that contains 'color:#xxxxxx;'
    // 2) the file uses 'fill' attribute with explicit colors.

    // in case 1) we replace only CSS styled colors, in case 2) we replace color in all 'fill'
    // attributes
    // this code probably needs to be re-written using XML parser if the number of cases increases

    const bool svgContainsCSSStyles = svgContents.find("style type=\"text/css\"") != std::string::npos;

    res << std::regex_replace(svgContents,
                                svgContainsCSSStyles ? m_cssColorRegex : m_fillColorRegex,
                                "$1$2" + newColor.name().toStdString() + "$3");
}


UIThemeManager *UIThemeManager::m_instance = nullptr;

void UIThemeManager::initInstance()
{
    if (!m_instance)
        m_instance = new UIThemeManager;
}

UIThemeManager::UIThemeManager()
    : m_iconsTemporaryDir {temporaryDirForIcons()}
    , m_coloredIconsDir(m_iconsTemporaryDir.path() + QDir::separator() + QLatin1String("colorized"))
    , m_svgManipulator(new SVGManipulator())
{
    if (!m_coloredIconsDir.isValid())
        qCWarning(theme, "Could not create temporary directory '%s' for colorized icons", qUtf8Printable(m_coloredIconsDir.path()));

    const Preferences *const pref = Preferences::instance();
    if (pref->useCustomUITheme()
        && !QResource::registerResource(pref->customUIThemePath(), "/uitheme"))
        LogMsg(tr("Failed to load UI theme from file: \"%1\"").arg(pref->customUIThemePath()), Log::WARNING);

    connect(&Theme::ThemeProvider::instance(), &Theme::ThemeProvider::colorThemeChanged,
            this, &UIThemeManager::colorThemeChanged);
    update();
}

UIThemeManager::~UIThemeManager() = default;

UIThemeManager *UIThemeManager::instance()
{
    return m_instance;
}

void UIThemeManager::applyStyleSheet() const
{
    if (!Preferences::instance()->useCustomUITheme()) {
        qApp->setStyleSheet({});
        return;
    }

    QFile qssFile(":uitheme/stylesheet.qss");
    if (!qssFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qApp->setStyleSheet({});
        LogMsg(tr("Couldn't apply theme stylesheet. stylesheet.qss couldn't be opened. Reason: %1").arg(qssFile.errorString())
               , Log::WARNING);
        return;
    }

    qApp->setStyleSheet(qssFile.readAll());
}

QIcon UIThemeManager::getIcon(const QString &iconId) const
{
    return getIcon(iconId, iconId);
}

QIcon UIThemeManager::getIcon(const QString &iconId, const QString &fallback) const
{
const auto it = m_generatedIcons.find(iconId);
    if (it != m_generatedIcons.end()) {
        return it.value();
    }
#if (defined(Q_OS_UNIX) && !defined(Q_OS_MAC))
    if (iconSet() == IconSet::SystemTheme) {
        QIcon icon = QIcon::fromTheme(iconId);
        if (icon.isNull() || icon.name() != iconId)
            icon = QIcon::fromTheme(fallback, QIcon(IconProvider::getIconPath(iconId)));
        return icon;
    }
#else
    Q_UNUSED(fallback)
#endif
    // cache to avoid rescaling svg icons
    static QHash<QString, QIcon> iconCache;
    const auto iter = iconCache.find(iconId);
    if (iter != iconCache.end())
        return *iter;

    const QIcon icon {IconProvider::getIconPath(iconId)};
    iconCache[iconId] = icon;
    return icon;
}

QIcon UIThemeManager::getFlagIcon(const QString &countryIsoCode) const
{
    if (countryIsoCode.isEmpty()) return {};
    return QIcon(":/icons/flags/" + countryIsoCode.toLower() + ".svg");
}

QString UIThemeManager::getIconPath(const QString &iconId) const
{
#ifdef HAVE_KF5ICONTHEMES
    if (iconSet() == IconSet::SystemTheme)
        return KIconLoader::global()->iconPath(iconId, KIconLoader::Desktop);
#endif
    return IconProvider::getIconPath(iconId);
}

QIcon UIThemeManager::icon(BitTorrent::TorrentState state) const
{
    return m_torrentStateIcons.value(state);
}

UIThemeManager::IconSet UIThemeManager::iconSet()
{
    return iconSetSetting();
}

void UIThemeManager::setIconSet(UIThemeManager::IconSet v)
{
    if (iconSet() != v) {
        iconSetSetting() = v;
        instance()->update();
    }
}

bool UIThemeManager::stateIconsAreColorized()
{
    return stateIconsAreColorizedSetting();
}

void UIThemeManager::setStateIconsAreColorized(bool v)
{
    if (v != stateIconsAreColorized()) {
        stateIconsAreColorizedSetting() = v;
        instance()->update();
    }
}

void UIThemeManager::colorThemeChanged()
{
    update();
}

void UIThemeManager::reset()
{
    using BitTorrent::TorrentState;
    m_torrentStateIcons[TorrentState::Downloading] =
    m_torrentStateIcons[TorrentState::ForcedDownloading] =
    m_torrentStateIcons[TorrentState::DownloadingMetadata] = QIcon(":/icons/skin/downloading.svg");
    m_torrentStateIcons[TorrentState::Allocating] =
    m_torrentStateIcons[TorrentState::StalledDownloading] = QIcon(":/icons/skin/stalledDL.svg");
    m_torrentStateIcons[TorrentState::StalledUploading] = QIcon(":/icons/skin/stalledUP.svg");
    m_torrentStateIcons[TorrentState::Uploading] =
    m_torrentStateIcons[TorrentState::ForcedUploading] = QIcon(":/icons/skin/uploading.svg");
    m_torrentStateIcons[TorrentState::PausedDownloading] = QIcon(":/icons/skin/paused.svg");
    m_torrentStateIcons[TorrentState::PausedUploading] = QIcon(":/icons/skin/completed.svg");
    m_torrentStateIcons[TorrentState::QueuedDownloading] =
    m_torrentStateIcons[TorrentState::QueuedUploading] = QIcon(":/icons/skin/queued.svg");
    m_torrentStateIcons[TorrentState::CheckingDownloading] =
    m_torrentStateIcons[TorrentState::CheckingUploading] =
#if LIBTORRENT_VERSION_NUM < 10100
    m_torrentStateIcons[TorrentState::QueuedForChecking] =
#endif
    m_torrentStateIcons[TorrentState::CheckingResumeData] = QIcon(":/icons/skin/checking.svg");
    m_torrentStateIcons[TorrentState::Unknown] =
    m_torrentStateIcons[TorrentState::MissingFiles] =
    m_torrentStateIcons[TorrentState::Error] = QIcon(":/icons/skin/error.svg");
}

void UIThemeManager::update()
{
    if (iconSet() == IconSet::Monochrome) {
        decolorizeIcons();
        setIconDir(m_coloredIconsDir.path());
    }
    else {
        setIconDir(IconProvider::defaultIconDir());
    }

    m_generatedIcons[speedometerFlippedIconName] = flipIcon(getIcon(speedometerIconName), true, false);

    using BitTorrent::TorrentState;
    const auto trySetColorizedIcon = [this](BitTorrent::TorrentState state, const char *stateName, const char *iconName)
    {
        const QString iconPath = QString(QLatin1String(":/icons/skin/%1.svg")).arg(QLatin1String(iconName));
        const QString colorizedIconPath = QDir(this->m_coloredIconsDir.path())
            .filePath(QLatin1String(stateName) + QLatin1String(".svg"));
        try {
            m_svgManipulator->replaceSVGFillColor(iconPath, colorizedIconPath,
                                                  Theme::ColorTheme::current().torrentStateColor(state));
            m_torrentStateIcons[state] = QIcon(colorizedIconPath);
        }
        catch (std::runtime_error &ex) {
            qCWarning(theme, "Could not colorize icon '%s': '%s'", iconName, ex.what());
            m_torrentStateIcons[state] = QIcon(iconPath);
        }
    };

    if (stateIconsAreColorized() && m_coloredIconsDir.isValid()) {
        trySetColorizedIcon(TorrentState::Downloading, "torrent-state-downloading", "downloading");
        trySetColorizedIcon(TorrentState::ForcedDownloading, "torrent-state-forceddownloading", "downloading");
        trySetColorizedIcon(TorrentState::DownloadingMetadata, "torrent-state-downloadingmetadata", "downloading");
        trySetColorizedIcon(TorrentState::Allocating, "torrent-state-allocating", "stalledDL");
        trySetColorizedIcon(TorrentState::StalledDownloading, "torrent-state-stalleddownloading", "stalledDL");
        trySetColorizedIcon(TorrentState::StalledUploading, "torrent-state-stalleduploading", "stalledUP");
        trySetColorizedIcon(TorrentState::Uploading, "torrent-state-Uploading", "uploading");
        trySetColorizedIcon(TorrentState::ForcedUploading, "torrent-state-forceduploading", "uploading");
        trySetColorizedIcon(TorrentState::PausedDownloading, "torrent-state-pauseddownloading", "paused");
        trySetColorizedIcon(TorrentState::PausedUploading, "torrent-state-pauseduploading", "completed");
        trySetColorizedIcon(TorrentState::QueuedDownloading, "torrent-state-queueddownloading", "queued");
        trySetColorizedIcon(TorrentState::QueuedUploading, "torrent-state-queueduploading", "queued");
        trySetColorizedIcon(TorrentState::CheckingDownloading, "torrent-state-checkingdownloading", "checking");
        trySetColorizedIcon(TorrentState::CheckingUploading, "torrent-state-checkinguploading", "checking");
#if LIBTORRENT_VERSION_NUM < 10100
        trySetColorizedIcon(TorrentState::QueuedForChecking, "torrent-state-queuedforchecking", "checking");
#endif
        trySetColorizedIcon(TorrentState::CheckingResumeData, "torrent-state-checkingresumedata", "checking");
        trySetColorizedIcon(TorrentState::Unknown, "torrent-state-unknown", "error");
        trySetColorizedIcon(TorrentState::MissingFiles, "torrent-state-missingfiles", "error");
        trySetColorizedIcon(TorrentState::Error, "torrent-state-error", "error");
    }
    else {
        reset();
    }

    emit iconsChanged();
}

void UIThemeManager::decolorizeIcons()
{
    // process every svg file from :/icons/qbt-theme/
    QDir srcIconsDir(QLatin1String(":/icons/qbt-theme/"));
    const QStringList icons = srcIconsDir.entryList({QLatin1String("*.svg")}, QDir::Files);
    const QColor newIconColor = QPalette().color(QPalette::WindowText);
    for (const QString &iconFile: icons) {
        try {
            m_svgManipulator->replaceSVGFillColor(srcIconsDir.absoluteFilePath(iconFile),
                                                  m_coloredIconsDir.path() + QDir::separator() + iconFile, newIconColor);
        }
        catch (std::runtime_error &ex) {
            qCWarning(theme, "Could not colorize icon '%s': '%s'", qPrintable(iconFile), ex.what());
        }
    }
}

QString UIThemeManager::temporaryDirForIcons()
{
    // TODO Application should create QTemporaryDir for /run/user/<USERID>/<ApplicationName>/<PID>
    // and we then will be able to create directory "icons" inside

    QString runPath = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (runPath.isEmpty()) runPath = QDir::tempPath();

    return runPath + QDir::separator() +
        QCoreApplication::applicationName() +
        QLatin1String("_icons_");
}

CachedSettingValue<UIThemeManager::IconSet> &UIThemeManager::iconSetSetting()
{
    static CachedSettingValue<IconSet> setting("Appearance/IconSet", IconSet::Default);
    return setting;
}

CachedSettingValue<bool> &UIThemeManager::stateIconsAreColorizedSetting()
{
    static CachedSettingValue<bool> setting("Appearance/StateIconColorization", true);
    return setting;
}

QIcon UIThemeManager::flipIcon(const QIcon& icon, bool horizontal, bool vertical)
{
    QIcon res;
    const QList<QSize> sizes = !icon.availableSizes().isEmpty()
                        ? icon.availableSizes()
                        : QList<QSize>{{16, 16}, {32, 32}, {64, 64}, {128, 128}, {256, 256}};
    for (const auto &size : sizes) {
        QPixmap px {size};
        px.fill({0, 0, 0, 0});
        QPainter painter {&px};
        QRect bounds {0, 0, size.width(), size.height()};
        QTransform tr;
        tr.scale(horizontal ? -1. : 1., vertical ? -1. : 1.);
        tr.translate(horizontal ? -size.width() : 0, vertical ? -size.height() : 0);
        painter.setTransform(tr);
        painter.drawPixmap(0, 0, icon.pixmap(size));
        res.addPixmap(px);
    }
    return res;
}
