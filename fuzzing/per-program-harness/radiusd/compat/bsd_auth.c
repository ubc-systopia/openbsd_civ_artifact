#include <pwd.h>
#include <string.h>
#include <unistd.h>

int
auth_userokay(const char *user, const char *style, const char *type,
    const char *password)
{
	struct passwd *pw;
	char *hash;

	(void)style;
	(void)type;
	if ((pw = getpwnam(user)) == NULL || pw->pw_passwd == NULL ||
	    pw->pw_passwd[0] == '\0')
		return (0);
	if ((hash = crypt(password, pw->pw_passwd)) == NULL)
		return (0);
	return (strcmp(hash, pw->pw_passwd) == 0);
}
