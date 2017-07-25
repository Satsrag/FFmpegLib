//
// Created by Saqrag Borgn on 05/07/2017.
//

#ifndef SMALLVIDEOVIEW_COMPRESS_H
#define SMALLVIDEOVIEW_COMPRESS_H

int compress(
        const char *inFilename,
        const char *outFilename,
        long videoBitRate,
        long audioBitRate,
        int width,
        int height,
        int threadCount
);

void cancelCompress();

#endif //SMALLVIDEOVIEW_COMPRESS_H
