#!/usr/bin/env python

##
#  Retrieves the actual data files from USC/CARC
#

import getopt
import sys
import subprocess
import json

model = "SFCVM"

if sys.version_info.major >= (3) :
  from urllib.request import urlopen
else:
  from urllib2 import urlopen

def usage():
    print("\n./make_data_files.py\n\n")
    sys.exit(0)

def download_urlfile(url,fname):
  try:
    response = urlopen(url)
    CHUNK = 16 * 1024
    with open(fname, 'wb') as f:
      while True:
        chunk = response.read(CHUNK)
        if not chunk:
          break
        f.write(chunk)
  except:
    e = sys.exc_info()[0]
    print("Exception retrieving and saving model datafiles:",e)
    raise 
  return True

def main():

    # Set our variable defaults.
    path = ""
    bpath = ""
    deplist = ""

    try:
        fp = open('./config','r')
    except:
        print("ERROR: failed to open config file")
        sys.exit(1)

    ## look for model_data_path and other varaibles
    lines = fp.readlines()
    for line in lines :
        if line[0] == '#' :
          continue
        parts = line.split('=')
        if len(parts) < 2 :
          continue;
        variable=parts[0].strip()
        val=parts[1].strip()

        if (variable == 'model_data_path') :
            path = val + '/' + model
            bpath = val + '/' + 'SFCVM'
            continue
        if (variable == 'model_dir') :
            mdir = "./"+val
            bdir = "./"+"sfcvm"
            continue
        if (variable == 'model_dependency') :
            deplist = json.loads(val) 
            continue
        continue
    if path == "" :
        print("ERROR: failed to find variables from config file")
        sys.exit(1)

    fp.close()

    print("\nDownloading model dataset\n")

    subprocess.check_call(["mkdir", "-p", mdir])

## model's data file
    flist= [ 'oUSGS_SFCVM_v21-1_detailed.h5' ]

    for f in flist :
        fname = mdir + "/" +f
        url = path + "/" + fname
        print(url, fname)
        try: 
          download_urlfile(url,fname)
        except:
          sys.exit(1)

## model's dependencies
    klist= deplist.keys()

    for fname in klist :
        deppath=deplist[fname]
        newfname="../dependencies/"+fname
        url = deppath + "/" + fname
        print(url, newfname)
        try: 
          download_urlfile(url,newfname)
        except:
          sys.exit(1)

    print("\nDone!")

if __name__ == "__main__":
    main()
