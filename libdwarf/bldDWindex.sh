sed -n -e '/^%%Page.*/p' -e '/.*DW_.*/p' <dwarf.v2.ps  |
sed -n  -e '/^%%Page.*/p' -e 's/.*\(DW_[a-z_A-Z]*\).*/\1/p' |
nawk '  /^%%Page/{ p = $2  } \
	 /DW_/	{ printf "%-30s  %04d \n",$1,p \
	}'   | sort  -u  |
nawk ' BEGIN {h = "xx"}  		\
         {  				\
	    done = 0 ; 			\
            if ( $1 != h ) {     	\
	 	  if(h != "xx") { 	\
			printf "%-30s %s\n",h, pgs ; \
			h = $1 ;	\
			tv = $2 + 0 ;   \
			done = 1  ;     \
			pgs = "" tv 	\
 		  }		 	\
	     }				\
	     h = $1 ;                   \
	     if(done == 0 ) {  		\
		tv = $2 + 0 ;           \
		pgs = pgs ", "  tv      \
	     }    			\
	  } 				\
          END { printf "%-30s %s\n",h,pgs }  '  
