#ifndef SUBTITLE_H
#define SUBTITLE_H

#include <QString>
#include <QDebug>
#include <QFont>

#include "mpv/client.h"
#include "mpv/qthelper.hpp"

struct SubtitleInfo {
    int id;
    QString lang;
};

class Subtitle {
public:
    explicit Subtitle(mpv_handle *mpv);

    void loadSubtitle(const QString &filename);

    void setSubtitleTrack(int track);

    void setSubtitleDelay(double delay);

    void setSubtitleFont(const QString &font, int size);

    QFont getCurrentSubtitleFont();

    QMap<QString, SubtitleInfo> getSubtitleList();

private:

    mpv_handle *mpv;

};


#endif //SUBTITLE_H
