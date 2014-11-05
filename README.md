# AIP Science Apps Workspace

The Science Apps Workspace for the [Arabidopsis Information Portal][1].

## Drupal module

Provides a content type "Science App".

## Requirements

The app preparation/checkout expects that `bower` be in the `$PATH` of the user the portal is running as, e.g. apache or nginx. The user must be able to run
those commands.

If running as the apache/nginx user on a standard distribution, this user is
often unprivileged. It is probably no `nologin` account and has $HOME assigned
to `/var/www` for example, to which it may not have permissions. This can
cause some unexpected behavior. We have found that in these cases, rather
than granting more privileges to the user, manually working through the error
messages to deduce the correct configuration is best.

Common issues:

- `bower` and `yeoman` attempt to create directories like `$HOME/.config` or `$HOME/.cache`
  - resolution: manually create the directories and grant the user permission
- `bower`, `git`, etc., are not in the path
  - edit the path for the user. For CentOS httpd, edit `/etc/sysconfig/httpd` so that the `$PATH` is correct.


[1]: https://www.araport.org
