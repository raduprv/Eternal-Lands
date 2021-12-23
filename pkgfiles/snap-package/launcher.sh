#! /bin/sh

datadir=$SNAP/data
userdir=$SNAP_USER_COMMON/config
userdirlink=$SNAP_USER_DATA/.elc
serverfile=servers.lst
defserver=default_server_id.txt
inifile=el.ini
exename=$SNAP/bin/el.linux.bin
browser=xdg-open

mkdir -p $userdir || exit

ln -sf $userdir $userdirlink || exit

if [ ! -r $userdir/$serverfile ]
then
	cp -p $datadir/$serverfile $userdir/ || exit
fi

if [ -z "$1" ]
then
	if [ -r $userdir/$defserver ]
	then
		config="$(cat $userdir/$defserver)"
	else
		config="main"
	fi
else
	config="$1"
fi

configdir="$(grep ^${config} $userdir/$serverfile | awk {'print $2'})"
mkdir -p $userdir/$configdir || exit

if [ ! -r $userdir/$configdir/$inifile ]
then
	cp -p $datadir/$inifile $userdir/$configdir/ || exit
fi
chmod +wr $userdir/$configdir/$inifile

# if no TTF specified, or not a valid path, try to override
override_ttf=false
fft_dir_line="$(grep "^#ttf_directory" $userdir/$configdir/$inifile)"
if [ -z "$fft_dir_line" ]
then
	override_ttf=true
else
	ini_fft_path="$(echo "$fft_dir_line" | cut -d\= -f2 | cut -d\" -f2)"
	[ -d "$ini_fft_path" ] || override_ttf=true
fi

# if overriding TTF, look for a suitable path
ttfdir=""
if $override_ttf
then
	for dir in 	"/usr/share/fonts/truetype/" "/usr/share/fonts/TTF/" "/usr/share/fonts/"
	do
		if [ -d "$dir" ]
		then
			ttfdir="-ttfdir=$dir"
			break
		fi
	done
fi

exec "$exename" -dir="$datadir" -b="$browser" "$ttfdir" "$config"
