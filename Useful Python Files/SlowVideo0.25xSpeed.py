# Slows down a video with audio to 25% speed.

# You either need the ffmpeg-python installed or normal ffmpeg on your computer, I have both
# installed so I'm not sure which... I think it's ffmpeg-python.  

import os, sys
import ffmpeg

basePath = "G:/My Drive/Documents/SCHOOL/Thesis/Videos/"

fileNameNoExtension = "GforceNoWalkerAnkleWeights 2"           # File name in File Explorer
origName = basePath + fileNameNoExtension + ".mp4"             # File name with extension
slowVideoName = basePath + fileNameNoExtension + "_SLOW1.mp4"  # New name

try:
    stream = ffmpeg.input(origName)
    slowVideo = ffmpeg.setpts(stream,'4*PTS')  # Slow it down to 1/4 speed
    audio1 = stream.audio.filter('asetrate', 44100*0.25) #Change the pitch and speed of audio
    
    #Output video with sound 
    outputBoth = ffmpeg.output(audio1, slowVideo, slowVideoName)
    ffmpeg.run(outputBoth, 
               capture_stdout=True, 
               capture_stderr=True,
               overwrite_output=True
               )  
    
except ffmpeg.Error as e:
    print(e.stderr)

