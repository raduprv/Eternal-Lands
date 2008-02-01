use strict;

open IN, "<load_gl_extensions.h";

while (<IN>) {
 if (m/extern (PF.+?) EL/) {
  my $ext = $1;
  if (`grep $ext glext.h` =~ m/(typedef .+;)/) {
   my $type = $1;
   $type =~ s/APIENTRYP/\*/;
   print "#ifndef $ext\n"; 
   print " $type\n";
   print "#endif\n";
  }
 }
}
