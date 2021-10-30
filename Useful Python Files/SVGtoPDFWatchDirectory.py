# This file watches a directory and when new SVG files are added to the directory it
# automatically converts them to PDF.   Useful for if you make figures in PowerPoint 
# and export them as SVG so you get vector graphics, but then need to upload them to 
# Overleaf which doesn't support PDFs (and SVG packages don't always render the same)
# 
# Source for watch directory stuff:  
#        http://timgolden.me.uk/python/win32_how_do_i/watch_directory_for_changes.html
# Source for SVG to PDF:  
#        https://stackoverflow.com/questions/5835795/generating-pdfs-from-svg-input

import os, time
from svglib.svglib import svg2rlg
from reportlab.graphics import renderPDF

thisBasePath = "G:/My Drive/Documents/SCHOOL/Thesis/Figures/"

path_to_watch = thisBasePath
before = dict ([(f, None) for f in os.listdir (path_to_watch)])

while 1:
  time.sleep (10)  # Check for changes every 10 seconds
  after = dict ([(f, None) for f in os.listdir (path_to_watch)])
  added = [f for f in after if not f in before]
  removed = [f for f in before if not f in after]
  if (added): 
    print("Found new file: ", ", ".join(added))
    
    # Convert it to pdf if its svg
    if(added[0][-3:] == "svg"):
        drawing = svg2rlg(thisBasePath + added[0])
        renderPDF.drawToFile(drawing, thisBasePath + added[0][:-4] + ".pdf")
        print("Converted " + added[0] + " to PDF!")

  if removed: print("File was removed: ", ", ".join (removed))
  before = after
