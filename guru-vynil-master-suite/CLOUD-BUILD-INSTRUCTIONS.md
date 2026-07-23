# Build the Windows installer without development tools

You do **not** need Visual Studio, CMake or Git on your own computer. The included GitHub Actions workflow builds the plugin on a hosted Windows 2022 machine and returns a normal Windows installer.

## One-time setup

1. Sign in to GitHub and create a new empty repository.
2. Extract this project ZIP on your computer.
3. In the empty repository, choose **Add file → Upload files**.
4. Drag all files and folders from inside the extracted project folder into the browser. Make sure the `.github` folder is included.
5. Commit the uploaded files to the default branch.

## Produce the installer

1. Open the repository's **Actions** tab.
2. Select **Build Windows Installer**.
3. Choose **Run workflow**.
4. Keep the version as `0.1.1` and run it.
5. Open the completed workflow run.
6. Download the artifact named similar to:

   `GURU-Vynil-Master-Suite-0.1.1-Windows-x64`

7. Extract the downloaded artifact and run:

   `GURU-Vynil-Master-Suite-Setup-0.1.1-Windows-x64.exe`

## What the installer does

- Installs the VST3 bundle into the standard 64-bit Windows VST3 directory:

  `C:\Program Files\Common Files\VST3\GURU Vynil Master Suite.vst3`

- Installs the standalone application under Program Files.
- Adds an uninstall entry to Windows Apps & Features.
- Optionally creates a desktop shortcut.

## Windows security warning

This early build is unsigned. Windows SmartScreen may show an **Unknown publisher** warning. Code signing requires a separate commercial code-signing certificate. Only run an installer produced from your own repository workflow.
