
files=("static/index.html")
target="static.h"

echo "" > $target

for file in "${files[@]}"
do
   cat $file | sed 's/\s*/ /' > $file.wrzlbrnft
   
   xxd -i "$file.wrzlbrnft" | \
      sed 's/_wrzlbrnft//' | \
      sed 's/unsigned char/const PROGMEM char/' | \
      sed 's/};/, 0x00 };/' >> $target

   rm $file.wrzlbrnft
done
