ExplorerExtSilentRenaming
=========================


A hook that suppresses the confirmation when changing extensions in Windows Explorer.

This should work, language agnostically, from Windows 7 up to Windows 10, where SHMessageBox calls `TaskDialogIndirect`_, instead of MessageBoxW.

The latter case is that of Windows XP, which is not treated for the time being.

Tested on Windows 7, Windows 8.1 & Windows 10; en_US & zh_CN.


Head to the `source`_ if having appetite.


Setup Instructions
------------------

1. Download a release.
2. Extract files to a normal path, e.g., the user desktop or the root of your system disk.
3. Run the setup script as administrator.

   Or use your own way to inject the dll into explorer.exe.

4. Terminate all explorer.exe processes and create a new one with Task Manager.

   Alternatively, simply reboot your system.

5. Ready to go.


.. _source: ExplorerExtSilentRenamingMixIn/ExplorerExtSilentRenamingMixIn/ExplorerExtSilentRenamingMixIn_dllmain.cpp
.. _TaskDialogIndirect: https://docs.microsoft.com/en-us/windows/win32/api/commctrl/nf-commctrl-taskdialogindirect
