#! /bin/sh

datadir=$SNAP/data
userdir=$SNAP_USER_COMMON/config
userdirlink=$SNAP_USER_DATA/.elc
serverfile=servers.lst
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
	config="main"
else
	config="$1"
fi

mkdir -p $userdir/$config || exit

if [ ! -r $userdir/$config/$inifile ]
then
	cp -p $datadir/$inifile $userdir/$config/ || exit
fi

# if no TTF specified, or not a valid path, try to override
override_ttf=false
fft_dir_line="$(grep "^#ttf_directory" $userdir/$config/$inifile)"
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
