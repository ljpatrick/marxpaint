import sys, re, os, shutil, fnmatch

def lfcr(file):
    if os.path.isdir(file):
        print file, "Directory!"
        return
    data = open(file, "rb").read()
    if '\0' in data:
        print file, "Binary!"
        return
    newdata = re.sub("\r?\n", "\r\n", data)
    if newdata != data:
        print "Converting '"+file+"' to CRLF format."
        f = open(file, "wb")
        f.write(newdata)
        f.close()


def makeFolders(path):
    if not os.path.exists(path):
        os.makedirs(path)


def po2mo(pfx, input, output):
    output_path = os.path.join(output,pfx,"LC_MESSAGES")
    makeFolders(output_path)
    output_path = os.path.join(output_path,"tuxpaint.mo")
    os.system("msgfmt.exe -o"+output_path+" "+input+pfx+".po")

def add2filelist(fp, destination, source):
    absSource = os.path.abspath(source)
    for root, dirs, filenames in os.walk(source):
        fp.write("  SetOutPath $INSTDIR\\%s"%destination)
        relpath = os.path.abspath(root)[len(absSource)+1:]
        if relpath: fp.write("\\%s"%relpath)
        fp.write("\n")
        for file in filenames:
            fp.write('  File "%s"\n'%os.path.join(root, file))
        if 'CVS' in dirs: dirs.remove('CVS')  # don't visit CVS directories
        if 'cvs' in dirs: dirs.remove('cvs')

docs = "../docs/"
for doc in os.listdir(docs):
    fullpath = docs+doc
    if os.path.isdir(fullpath): continue
    lfcr(fullpath)

pos = "../src/po/"
mos = "locale"

makeFolders(mos)
for po in fnmatch.filter(os.listdir(pos), "*.po"):
    fullpath = pos+po
    if os.path.isdir(fullpath): continue
    po2mo(os.path.splitext(po)[0], pos, mos)

fp = open("filelist.inc", "w")
add2filelist(fp, "data", "..\\data")
add2filelist(fp, "data\\fonts", "..\\fonts")
add2filelist(fp, "data\\starters", "..\\starters")
add2filelist(fp, "data\\stamps", "..\\stamps")
add2filelist(fp, "docs", "..\\docs")
add2filelist(fp, "userdata", "userdata")
fp.write("\n")
fp.close()


