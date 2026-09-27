#!/bin/bash

ffmpeg -framerate 30 -i images/image00_%03d.bmp -c:v libx264 -preset veryslow -crf 32 -pix_fmt yuv420p output.mp4