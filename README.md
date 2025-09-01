# Simple-As Video Editor (SAVE)

Disclaimer: This project is extremely vibe-coded! I was just brainstorming when Claude suddenly spit out the whole project, so I thought I may as well give it a go. The code, and the README below, are largely written by Claude Sonnet 3.7. I can't guarantee functionality or coherency, nor that I'll be able to edit the code myself. We'll see, though!

A lightweight, intuitive video editor designed for simple editing tasks. Think of it as "MS Paint for video" - focused on easy access to common operations without the complexity of professional editing suites.

## Purpose

This application fills the gap between professional video editing software (which can be intimidating and complex) and overly simplified apps that lack essential functionality. SAVE provides a straightforward GUI for common FFMPEG operations, making it easy to perform basic video editing tasks without requiring command-line knowledge.

## Features

- **Trimming**: Cut segments from the beginning or end of videos
- **Cropping**: Visually select and crop specific regions of the video frame
- **Resizing**: Scale videos to standard resolutions or custom dimensions
- **Format Conversion**: Convert between common video formats with appropriate codec selection
- **Simple Playback Controls**: Preview videos before and after editing
- **Clean, Intuitive Interface**: Focused on simplicity and ease of use

## Dependencies

- Qt 5.15+ (for GUI components)
- FFMPEG (for video processing)

## Building the Project

### Prerequisites

1. Install Qt development environment:
   - Download from [Qt.io](https://www.qt.io/download) or use your package manager
   - Make sure to include Qt Multimedia components

2. Install FFMPEG:
   - **Linux**: `sudo apt install ffmpeg` (Ubuntu/Debian) or equivalent
   - **macOS**: `brew install ffmpeg`
   - **Windows**: Download from [ffmpeg.org](https://ffmpeg.org/download.html) and add to PATH

### Compilation

```bash
# Clone the repository
git clone https://github.com/AlphaKretin/simple-as-video-editor.git
cd simple-as-video-editor

# Generate build files with qmake
qmake

# Build the project
make  # Linux/macOS
# or
nmake  # Windows with MSVC
```

## Project Structure

- **main.cpp**: Application entry point
- **SimpleVideoEditor.h/cpp**: Main window and application logic
- **TrimDialog.h/cpp**: UI for video trimming functionality
- **CropDialog.h/cpp**: UI for video cropping functionality
- **ResizeDialog.h/cpp**: UI for video resizing functionality
- **ConvertDialog.h/cpp**: UI for format conversion functionality

## Development Notes

This project uses FFMPEG as the backend for all video processing operations. The Qt GUI serves as a wrapper around common FFMPEG commands, making them accessible through a visual interface.

Key design principles:
1. **Simplicity First**: Focused on making common tasks easy rather than providing every possible option
2. **Visual Feedback**: Where possible, operations provide visual selection and preview
3. **FFMPEG Integration**: Leverages the power and compatibility of FFMPEG for actual processing
4. **Clean Architecture**: Separate dialog classes for each operation to maintain code organization

## Usage Examples

### Trimming a Video

1. Open a video file using File → Open or the keyboard shortcut
2. Click the "Trim" button
3. Set the start and end points using the time inputs
4. Click "OK" and choose where to save the trimmed video

### Converting Video Formats

1. Open a video file
2. Click the "Convert" button
3. Select the desired output format and quality settings
4. Click "OK" and choose where to save the converted video

## Future Enhancements

Potential features for future versions:
- Basic audio adjustments (volume, mute)
- Simple text overlay
- Basic filters (brightness, contrast)
- Batch processing of multiple files
- Speed adjustment (slow motion, timelapse)

## License

[GPLv3](LICENSE)

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.