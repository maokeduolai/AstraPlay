#include "subtitle.h"

Subtitle::Subtitle(mpv_handle *mpv) : mpv(mpv) {}

// 加载字幕
void Subtitle::loadSubtitle(const QString &filename) {
    const char *cmd[] = {"sub-add", filename.toUtf8().constData(), nullptr};
    mpv_command(mpv, cmd);
}

// 设置使用的字幕
void Subtitle::setSubtitleTrack(int track) {
    mpv::qt::set_property(mpv, "sid", track);
}

// 设置字幕同步
void Subtitle::setSubtitleDelay(double delay) {
    QVariant QNowDelay = mpv::qt::get_property_variant(mpv, "sub-delay");
    const double nowDelay = QNowDelay.toDouble();
    mpv::qt::set_property(mpv, "sub-delay", nowDelay + delay);
}

// 设置字幕字体字号
void Subtitle::setSubtitleFont(const QString &font, int size) {
    mpv::qt::set_property(mpv, "sub-font", font);
    mpv::qt::set_property(mpv, "sub-font-size", size);
}

// 获取当前字幕字体属性
QFont Subtitle::getCurrentSubtitleFont() {
    // 获取当前的字体字号
    QVariant QCurrentFontName = mpv::qt::get_property_variant(mpv, "sub-font");
    QString currentFontName = QCurrentFontName.toString();
    QVariant QCurrentFontSize = mpv::qt::get_property_variant(mpv, "sub-font-size");
    int currentFontSize = QCurrentFontSize.toInt();

    QFont currentFont;
    currentFont.setFamily(currentFontName);
    currentFont.setPointSize(currentFontSize);

    return currentFont;
}

// 获取字幕列表
QMap<QString, SubtitleInfo> Subtitle::getSubtitleList() {
    QMap<QString, SubtitleInfo> subtitleMap;

    mpv_node result;
    mpv_get_property(mpv, "track-list", MPV_FORMAT_NODE, &result);

    if (result.format == MPV_FORMAT_NODE_ARRAY) {
        for (int i = 0; i < result.u.list->num; i++) {
            mpv_node *node = &result.u.list->values[i];
            if (node->format == MPV_FORMAT_NODE_MAP) {
                // 分类取出对应值
                QString type;
                QString id;
                QString lang;
                QString title;
                for (int j = 0; j < node->u.list->num; j++) {
                    QString key(node->u.list->keys[j]);
                    mpv_node *value = &node->u.list->values[j];
                    if (key == "type" && value->format == MPV_FORMAT_STRING) {
                        type = value->u.string;
                    } else if (key == "id" && value->format == MPV_FORMAT_INT64) {
                        id = QString::number(value->u.int64);
                    } else if (key == "lang" && value->format == MPV_FORMAT_STRING) {
                        lang = value->u.string;
                    } else if (key == "title" && value->format == MPV_FORMAT_STRING) {
                        title = value->u.string;
                    }
                }
                if (type == "sub") {
                    SubtitleInfo info;
                    info.id = id.toInt();
                    info.lang = lang;

                    subtitleMap.insert(title, info);
                }
            }
        }
    }

    mpv_free_node_contents(&result);

    return subtitleMap;
}