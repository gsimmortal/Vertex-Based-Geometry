
#include "stdafx.h"
#include <glew\glew.h>
#include <iostream>

using namespace std;


void reportContextVersion(void) {

	int majorVersion, minorVersion;

	glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
	glGetIntegerv(GL_MINOR_VERSION, &minorVersion);

	cout << "OpenGL version " << majorVersion << "." << minorVersion << "\n\n";
}


void reportExtensions(void) {

	cout << "Extensions supported...\n\n";

	const char *glExtensionString = (const char *)glGetString(GL_EXTENSIONS);

	char *strEnd = (char*)glExtensionString + strlen(glExtensionString);
	char *sptr = (char*)glExtensionString;

	while (sptr < strEnd) {

		int slen = (int)strcspn(sptr, " ");
		printf("%.*s\n", slen, sptr);
		sptr += slen + 1;
	}
}