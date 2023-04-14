import subprocess
import sys
import os



if (os.path.exists("build")):
	dl=[]
	for r,ndl,fl in os.walk("build"):
		r=r.replace("\\","/").strip("/")+"/"
		for d in ndl:
			dl.insert(0,r+d)
		for f in fl:
			os.remove(r+f)
	for k in dl:
		os.rmdir(k)
else:
	os.mkdir("build")
if ("--release" in sys.argv):
	fl=[]
	for r,_,cfl in os.walk("src"):
		r=r.replace("\\","/").strip("/")+"/"
		for f in cfl:
			if (f[-2:]==".c"):
				fl.append(f"build/{(r+f).replace('/','$')}.o")
				if (subprocess.run(["gcc","-Wall","-lm","-Werror","-I","src/include","-O3","-c",r+f,"-o",f"build/{(r+f).replace('/','$')}.o"]).returncode!=0):
					sys.exit(1)
	if (subprocess.run(["gcc","-o","build/pico_usb_console"]+fl).returncode!=0):
		sys.exit(1)
else:
	fl=[]
	for r,_,cfl in os.walk("src"):
		r=r.replace("\\","/").strip("/")+"/"
		for f in cfl:
			if (f[-2:]==".c"):
				fl.append(f"build/{(r+f).replace('/','$')}.o")
				if (subprocess.run(["gcc","-Wall","-lm","-Werror","-I","src/include","-O0","-c",r+f,"-o",f"build/{(r+f).replace('/','$')}.o"]).returncode!=0):
					sys.exit(1)
	if (subprocess.run(["gcc","-o","build/pico_usb_console"]+fl).returncode!=0):
		sys.exit(1)
if ("--run" in sys.argv):
	subprocess.run(["build/pico_usb_console"])
