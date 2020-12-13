#!/bin/bash

masked=true
svg=false
jpg=false
aspect=false
trans="none"
fuzz="0%"

while [[ "$1" != "" ]]; do
    case "$1" in
        --nomask)
            masked=false
        ;;
        --aspect)
           aspect=true
        ;;
        --svg)
            svg=true
        ;;
        --jpg)
            jpg=true
        ;;
        --trans)
            trans="$2"
            if [[ "$trans" == "emoji" ]]; then
                trans="#ffcc4d"
            fi
            shift 1
        ;;
        --fuzz)
            fuzz="$2"
            shift 1
        ;;
        *)
            break
        ;;
    esac
    shift 1
done

if $svg; then
#    convert -size 128x128 "$1" -transparent "#FFFFFF" "$1.png" 2>&1 1>/dev/null
    sed -i 's/<\w*\s\+fill="#\(FFCC4D\|FFCB4C\|FFDC5D\)"[^\/]*\/\s*>//gI' "$1" 2>&1 1>/dev/null
    convert -size 112x112\! -background None "$1" "$1.png" 2>&1 1>/dev/null
    if [[ "$?" -ne "0" ]]; then
        exit 1
    fi
    set -- "$1.png" "$2"
elif $jpg; then
    convert "$1" -colorspace RGB "$1.png"
    set -- "$1.png" "$2"
fi

if [[ "$trans" != "none" ]]; then
    convert "$1" -fuzz "$fuzz" -transparent "$trans" "$1" 2>&1 1>/dev/null
fi

aspects="-resize 112x112"
aspects2="-geometry +4+0"
if $aspect; then
    aspects="-resize 128x128"
    aspects2=""
fi

if $masked; then
    convert -size 128x128 puffy/puff_template.png \( "$1" -gravity center $aspects \) $aspects2 -composite \( puffy/puffermask.png -alpha Off -compose CopyOpacity \) -composite -gravity center "$2" 2>&1 1>/dev/null
else
    convert -size 128x128 puffy/puff_template.png \( "$1" -gravity center $aspects \) $aspects2 -composite -gravity center "$2" 2>&1 1>/dev/null
fi

if $svg; then
    rm "$1" 2>/dev/null
fi
