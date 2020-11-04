#!/bin/bash

#local masked=true
svg=false
jpg=false

while [[ "$1" != "" ]]; do
    case "$1" in
#        --nomask)
#            masked=false
#        ;;
        --svg)
            svg=true
        ;;
        --jpg)
            jpg=true
        ;;
        *)
            break
        ;;
    esac
    shift 1
done

if $svg; then
    convert -size 512x512\! "$1" -transparent "#FFFFFF" "$1.png" 2>&1 1>/dev/null
    if [[ "$?" -ne "0" ]]; then
        exit 1
    fi
    set -- "$1.png" "$2"
elif $jpg; then
    convert "$1" -colorspace RGB "$1.png"
    set -- "$1.png" "$2"
fi
convert -size 512x548 xc:none -alpha set -channel RGBA \( "$1" -resize 512x512\! \) -composite "$1" 2>&1 1>/dev/null
convert "$1" -gravity center -matte -virtual-pixel transparent -distort Perspective '0,0 0,0 512,0 475,39 512,512 465,548 0,512 21,462' "$1" 2>&1 1>/dev/null
#    convert -size 728x728 "$1" -matte -virtual-pixel transparent -distort Perspective '0,0 0,0 0,512 475,39 512,512 464,548 512,0 21,462' "$1" 2>&1 1>/dev/null
#fi

convert puffy/puff3d_template.png \( "$1" -gravity center \) -geometry -46+19 -composite "$2" 2>&1 1>/dev/null

if $svg; then
    rm "$1" 2>/dev/null
fi
