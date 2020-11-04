#!/bin/bash

masked=true
svg=false
jpg=false

while [[ "$1" != "" ]]; do
    case "$1" in
        --nomask)
            masked=false
        ;;
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
    convert -size 128x128 "$1" -transparent "#FFFFFF" "$1.png" 2>&1 1>/dev/null
    if [[ "$?" -ne "0" ]]; then
        exit 1
    fi
    set -- "$1.png" "$2"
elif $jpg; then
    convert "$1" -colorspace RGB "$1.png"
    set -- "$1.png" "$2"
fi

if $masked; then
    convert -size 128x128 puffy/puff_template.png \( "$1" -gravity center -resize 128x128 \) -composite \( puffy/puffermask.png -alpha Off -compose CopyOpacity \) -composite -gravity center "$2" 2>&1 1>/dev/null
else
    convert -size 128x128 puffy/puff_template.png \( "$1" -gravity center -resize 128x128 \) -composite -gravity center "$2" 2>&1 1>/dev/null
fi

if $svg; then
    rm "$1" 2>/dev/null
fi
