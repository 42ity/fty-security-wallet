# Customized (manually maintained) bits of configure.ac for fty-warranty

AC_DEFUN([AX_PROJECT_LOCAL_HOOK], [
    AC_MSG_WARN([Running the PROJECT_LOCAL_HOOK])

    AC_ARG_WITH([socketSecurityWallet],
        [AS_HELP_STRING([--with-socketSecurityWallet=PATH], [Filename of fty-security-wallet service Unix socket])],,
        [with_socketSecurityWallet=/run/fty-security-wallet/secw.socket])
    AC_SUBST([socketSecurityWallet], [$with_socketSecurityWallet])
    AC_SUBST([socketSecurityWalletDir], [`dirname $with_socketSecurityWallet`])

])

AC_DEFUN([AX_PROJECT_LOCAL_HOOK_CONFIGVARS], [
    # Note: DO NOT refer to complete token name of this routine in the
    # message below, or you would get an infinite loop in m4 autoconf ;)
    AC_MSG_WARN([Running the PROJECT_LOCAL_HOOK_CONFIGVARS])

    ### Customization follows, to avoid verbatim '${prefix}' in generated files:
    dnl expand ${sysconfdir} and write it out
    conftemp="${sysconfdir}"
    eval conftemp=\"${conftemp}\"
    eval conftemp=\"${conftemp}\"
    CONFPATH=${conftemp}
    AC_DEFINE_UNQUOTED(CONFPATH, "${conftemp}", [Default path for configuration files])
    AC_SUBST(CONFPATH)

    dnl same for datadir
    conftemp="${datadir}"
    eval conftemp=\"${conftemp}\"
    eval conftemp=\"${conftemp}\"
    DATADIR=${conftemp}
    AC_DEFINE_UNQUOTED(DATADIR, "${conftemp}", [Default path for data files])
    AC_SUBST(DATADIR)

    dnl same for bindir
    conftemp="${bindir}"
    eval conftemp=\"${conftemp}\"
    eval conftemp=\"${conftemp}\"
    BINDIR=${conftemp}
    AC_DEFINE_UNQUOTED(BINDIR, "${conftemp}", [Default path for user executables])
    AC_SUBST(BINDIR)

    dnl same for sbindir
    conftemp="${sbindir}"
    eval conftemp=\"${conftemp}\"
    eval conftemp=\"${conftemp}\"
    SBINDIR=${conftemp}
    AC_DEFINE_UNQUOTED(SBINDIR, "${conftemp}", [Default path for system executables])
    AC_SUBST(SBINDIR)

    dnl same for libdir
    conftemp="${libdir}"
    eval conftemp=\"${conftemp}\"
    eval conftemp=\"${conftemp}\"
    LIBDIR=${conftemp}
    AC_DEFINE_UNQUOTED(LIBDIR, "${conftemp}", [Default path for system libraries])
    AC_SUBST(LIBDIR)

    dnl same for libexecdir
    conftemp="${libexecdir}"
    eval conftemp=\"${conftemp}\"
    eval conftemp=\"${conftemp}\"
    LIBEXECDIR=${conftemp}
    AC_DEFINE_UNQUOTED(LIBEXECDIR, "${conftemp}", [Default path for system libraries - executable helpers])
    AC_SUBST(LIBEXECDIR)

    dnl same for prefix
    conftemp="${prefix}"
    eval conftemp=\"${conftemp}\"
    eval conftemp=\"${conftemp}\"
    PREFIXDIR=${conftemp}
    AC_DEFINE_UNQUOTED(PREFIXDIR, "${conftemp}", [Default root path for component])
    AC_SUBST(PREFIXDIR)

    eval socketSecurityWallet=$socketSecurityWallet
    eval socketSecurityWalletDir=$socketSecurityWalletDir
])

AC_DEFUN([AX_PROJECT_LOCAL_HOOK_FINAL], [
    AC_MSG_WARN([Running the PROJECT_LOCAL_HOOK_FINAL])

    AC_CONFIG_FILES([
        src/fty-security-wallet.conf
    ])
])
