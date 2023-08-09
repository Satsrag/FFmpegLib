//
// Created by Saqrag Borgn on 05/07/2017.
//

#ifndef SMALLVIDEOVIEW_COMPRESS_H
#define SMALLVIDEOVIEW_COMPRESS_H

#include <stdint.h>

int compressF(
        int inFd,
        int64_t offset,
        int64_t length,
        int outFd,
        int64_t videoBitRate,
        int64_t audioBitRate,
        int width,
        int height,
        int videoId
);

void cancelCompress();

#endif //SMALLVIDEOVIEW_COMPRESS_H
