name			$(NAME)
version			$(VERSION)-1
architecture	$(ARCH)
summary 		"Open With Tracker Add-on"
description 	"Opens the selected file with an app symlinked into the \"Open With\" settings folder."
packager		"ablyss <jb@epluribusunix.net>"
vendor			"ablyss AI assisted project"
licenses {
	"MIT"
}
copyrights {
	"$(YEAR) ablyss"
}
provides {
	$(NAME) = $(VERSION)-1
}
requires {
	haiku
}
urls {
	"https://github.com/ablyssx74/OpenWith"
}
source-urls {
# Download
	"https://github.com/ablyssx74/OpenWith/archive/refs/tags/v.1.0.0.tar.gz"
}
