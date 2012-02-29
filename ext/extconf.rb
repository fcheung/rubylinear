require 'mkmf'
CONFIG["LDSHARED"] = "g++ -shared"
$CFLAGS = "#{ENV['CFLAGS']} -Wall -O3 -Werror"
create_makefile('rubylinear_native')
