#
#
#

BEGIN {
	
}


END {

}




	 {
	 	line = $0;
  	gsub(/\<p\>Definition at line \<a /, "<p class=\"def_at_line\" >Definition at line <a ", line);
  	gsub(/\<p\>References \<a /, "<p class=\"ref_to\" >References <a ", line);
  	gsub(/\<p\>Referenced by \<a /, "<p class=\"ref_from\" >Referenced by <a ", line);
  	
  	printf("%s\n", line);
}


