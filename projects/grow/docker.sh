#!/bin/sh

docker run \
        --rm \
        -v $(pwd)/../../:/project \
        -it espressif/idf:release-v3.3
