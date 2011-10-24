

import sys, os.path

browserok=1
try:
	import webbrowser
except:
	browserok=0
	pass

def showhelp():
	if browserok==0:
		print "Webbrowser module not found. Python probably to old."
	else:
		webbrowser.open_new("file://"+os.path.abspath(os.path.dirname(sys.argv[0]))+"/../doc/cereshelp.html")


if len(sys.argv)>1 and (sys.argv[1]=="--help" or sys.argv[1]=="-h" or sys.argv[1]=="-help"):
    print
    print "Usage: %s [-c configfile] [window-size] [infile]" % sys.argv[0]
    print
    print "-c configfile: If spesified, 'configfile' is used instead of '~/.ceres'."
    print
    print "window-size: 512, 1024, 2048 or 4096"
    print "             If not spesified, the default value in '~/.ceres' is used."
    print "             If window-size is neither spesified in '~/.ceres', nor"
    print "             on the command-line, the default-size of 1024 is used"
    print
    print "infile: All filetypes handled by libsndfile is supported."
    print "        Ceres is able to handle 8bit/16bit/24bit/32bit/float/double frametypes"
    print "        and up to 8 channels"
    print
    sys.exit(0)


conffile=os.path.expanduser("~/.ceres")
try:
    confnum=sys.argv.index("-c")
    try:
        conffile=os.path.expanduser(sys.argv.pop(confnum+1))
    except:
        conffile=os.path.expanduser("~/.ceres")
    sys.argv.remove("-c")
except:
    pass


#sys.path.append(os.path.dirname(os.environ["_"])+"/../lib/")
sys.path.insert(0,os.path.abspath(os.path.dirname(sys.argv[0]))+"/../lib/")
#print sys.argv[0]
#print os.path.abspath(sys.argv[0]+"/../lib/")
import ceresc
import ceresconfig

ceresconfig.ceresConfigStart(conffile)




windowsize=ceresconfig.getVar("ws_512")*512
if windowsize==0:
    windowsize=ceresconfig.getVar("ws_1024")*1024
if windowsize==0:
    windowsize=ceresconfig.getVar("ws_2048")*2048
if windowsize==0:
    windowsize=ceresconfig.getVar("ws_4096")*4096




for arg in sys.argv:
    if arg=="512" or arg=="1024" or arg=="2048" or arg=="4096":
        windowsize=int(arg)
        sys.argv.remove(arg)
        break
        


if len(sys.argv)>1:
    infile=sys.argv[1]
else:
    infile="++////bzzzt_nofile"
    


ceresc.CONFIG_init(ceresconfig.getVar,ceresconfig.showConfig)

ceresc.ceresmain(windowsize,infile)

