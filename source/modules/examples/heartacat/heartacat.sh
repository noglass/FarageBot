#!/bin/bash

masked=true
svg=false
jpg=false
aspect=true
trans="none"
fuzz="0%"
bg="#6e6e6e"
fancy=false
festive=false
hcolor="#b83346"
altcolor="white"
stripecolor="#282828"
input=true
del=true
nohrt=false
nude=true
nudecolor='#\(FFCC4D\|FFCB4C\|FFDC5D\)'
outline=false

while [[ "$1" != "" ]]; do
    case "$1" in
        --nomask)
            masked=false
        ;;
        --aspect)
           aspect=false
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
        --bg)
            bg="$2"
            if [[ "$bg" == "emoji" ]]; then
                bg="#ffcc4d"
            fi
            shift 1
        ;;
        --fancy)
            fancy=true
        ;;
        --festive)
            festive=true
        ;;
        --heart)
            hcolor="$2"
            if [[ "$hcolor" == "emoji" ]]; then
                hcolor="#ffcc4d"
            fi
            shift 1
        ;;
        --alt)
            altcolor="$2"
            if [[ "$altcolor" == "emoji" ]]; then
                altcolor="#ffcc4d"
            fi
            shift 1
        ;;
        --stripe)
            stripecolor="$2"
            if [[ "$stripecolor" == "emoji" ]]; then
                stripecolor="#ffcc4d"
            fi
            shift 1
        ;;
        --nonude)
            nude=false
        ;;
        --nude)
            nudecolor="$2"
            shift 1
        ;;
        --nodel)
            del=false
        ;;
        --nohrt)
            nohrt=true
        ;;
        --outline)
            outline=true
        ;;
        *)
            break
        ;;
    esac
    shift 1
done

if [[ "$1" == "null" ]]; then
    input=false
    svg=false
    jpg=false
fi

if $svg; then
#    convert -size 128x128 "$1" -transparent "#FFFFFF" "$1.png" 2>&1 1>/dev/null
    if $nude; then
        sed -i 's/<\w*\s\+fill="'${nudecolor}'"[^\/]*\/\s*>//gI' "$1" 2>&1 1>/dev/null
    fi
    convert -size 303x303\! -background None "$1" "$1.png" 2>&1 1>/dev/null
    if [[ "$?" -ne "0" ]]; then
        exit 1
    fi
    rm "$1" 2>/dev/null
    set -- "$1.png" "$2"
elif $jpg; then
    convert "$1" -colorspace RGB "$1.png"
    rm "$1" 2>/dev/null
    set -- "$1.png" "$2"
fi

overlay="heartacat/assets/maskedoverlay.png"
hearts="heartacat/assets/hrts.png"
if $nohrt; then
    overlay="heartacat/assets/overlay.png"
    hearts="heartacat/assets/smolhrt.png"
fi
convert -size 512x512 xc:none \( heartacat/assets/bg.png -fill $bg -colorize 100% \) \( heartacat/assets/alt.png -fill $altcolor -colorize 100% \) \( heartacat/assets/stripes.png -fill $stripecolor -colorize 100% \) \( $hearts -fill $hcolor -colorize 100% \) -background None -flatten $overlay -composite "$2" 2>&1 1>/dev/null

if $input; then
    args=( "$1" )
    if [[ "$trans" != "none" ]]; then
        args=( "$1" -fuzz $fuzz -transparent $trans )
    fi
    if $aspect; then
        args=( \( -background None "${args[@]}" -resize 303x303^ -extent 303x303 )
    else
        args=( \( -background None "${args[@]}" -resize 303x303 )
    fi
    if $outline; then
        args=( \( -background None -size 310x310 xc:none -gravity center "${args[@]}" -compose Over \) -composite \( -clone 0 -alpha extract -threshold 0 \) \( -clone 1 -blur 4x65000 -threshold 0 \) \( -clone 2 -fill black -opaque white \) \( -clone 3 -clone 0 -clone 1 -alpha off -compose over -composite \) -delete 0,1,3 +swap -alpha off -compose CopyOpacity -composite \) -gravity NorthWest -geometry +43+257 -compose Over -composite )
    else
        args=( "${args[@]}" \) -gravity NorthWest -geometry +43+257 -compose Over -composite )
    fi
    if $masked; then
        args=( "$2" "${args[@]}" \( heartacat/assets/bigheartmask.png -alpha Off -compose CopyOpacity \) -composite \( "$2" -compose DstOver \) -composite )
        if ! $nohrt; then
            args=( "${args[@]}" \( heartacat/assets/heartoverlay.png -compose Over \) -composite )
        fi
    else
        args=( -background None -size 512x512 xc:none "${args[@]}" \( +clone -alpha extract heartacat/assets/pawmask.png -compose CopyOpacity \) -composite )
        if ! $nohrt; then
            args=( "${args[@]}" \( heartacat/assets/heartoverlay.png -compose DstOver \) -composite )
        fi
        args=( "${args[@]}" \( "$2" -compose DstOver \) -composite )
    fi
    args=( "${args[@]}" \( heartacat/assets/pawoverlay.png -compose Over \) -composite \( heartacat/assets/nooverlay.png -compose Over \) -composite )
elif $nohrt; then
    args=( "$2" \( heartacat/assets/nooverlay.png -compose Over \) -composite \( heartacat/assets/pawoverlay.png -compose Over \) -composite )
else
    args=( "$2" \( heartacat/assets/heartoverlay.png -compose Over \) -composite \( heartacat/assets/pawoverlay.png -compose Over \) -composite )
fi

if $festive; then
    args=( "${args[@]}" \( heartacat/assets/festive.png -compose Over \) -composite )
fi

if $fancy; then
    args=( "${args[@]}" \( heartacat/assets/fancy.png -compose Over \) -composite )
fi

convert "${args[@]}" "$2" 2>&1 1>/dev/null

if $del; then
    rm "$1" 2>/dev/null
fi

