#!/bin/bash

ffmpeg -framerate 30 -i image00_%03d.bmp -c:v libx264 -pix_fmt yuv420p output.mp4