import os
import json
import platform 

# linux, xenomai, preempt-rt
default_config = {
	"rt_linux"	: "linux",
	"rt_skin"	: None
}

my_config = {
	"rt_linux"	: None ,
	"rt_skin"	: None	
}

def screen_clear():
	if os.name == 'nt' :
		os.system('cls')
	else :
		os.system('clear')

# check if Xenomai
def kernel_is_xenomai():
	return os.path.isdir("/proc/xenomai")

# check if preempt_rt
def kernel_is_preempt_rt():
	if 'PREEMPT RT' in platform.version() :
		return True
	else :
		return False

def check_kernel():
	global my_config
	screen_clear()

	if kernel_is_xenomai() is True:
		xeno_skin = 0
		my_config["rt_linux"] = "xenomai"
		print("Current kernel is Xenomai!\n\aSelect Xenomai Skin:\n\t1. Native\n\t2. POSIX")
		
		while xeno_skin != 1 and xeno_skin != 2:
			xeno_skin = int(input("Enter selection: "))
			if xeno_skin is 1:
				my_config["rt_skin"] = "native"
			elif xeno_skin is 2:
				my_config["rt_skin"] = "posix"
			else:
				print("Wrong Selection! Try Again!")

	elif kernel_is_preempt_rt() is True:
		my_config["rt_linux"] = "preempt-rt"
		my_config["rt_skin"] = None
	else:
		print "not a real-time kernel"
		my_config = default_config 

if __name__ == "__main__":
	import sys
	argc = len(sys.argv)

	if argc <= 1 :
		sys.exit("RT_AIDE root path required!\n Run setup.bash first!")

	root_path = sys.argv[1]
	config_path = root_path + "configs/"
	config_json_file = "config.json"
	config_json_fullpath = config_path + config_json_file

	if os.path.isfile(config_json_fullpath):
		with open(config_json_fullpath, 'r') as f:
			my_config = json.load(f)
			print("Old config.json:")
			print(json.dumps(my_config, indent=4))
		f.close()

	check_kernel()
	screen_clear()
	print "New config.json contents :"
	print(json.dumps(my_config, indent=4))
	print "\nWriting",config_json_fullpath,"...",

	with open(config_json_fullpath, 'w') as f:
		json.dump(my_config, f, indent=4)
	f.close()
	print("DONE!")