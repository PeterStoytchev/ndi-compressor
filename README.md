# ndi-compressor
Compresses a NDI source and outputs it to a ndi-server (another repo you can find on my account). Effectevly, this is a hacky implementation for NDI-HX, but it most certanly won't be as good compared to a hardware encoder, or if NewTek decide to release an actual NDI-HX software encoder (PLEASE DO IT).

You can think of the whole system, as an adapter, that sits in-between an NDI source and viewer. It captures the signal from the NDI source, compresses it (using ffmpeg libraries), sends it across the network (sockpp) to a server which will distribute it to ndi-compressor(s) which decompress it and turns it back into a NDI source which viewers can view. This enables NDI over low bandwidth networks, such as Wi-Fi or WAN, without the need for NDI-HX hardware or services which, essentailly, do the same thing (compress the signal to reduce bandwith).

Keep in mind that this is a hobby project created by a novice. Do not expect miracles. That said, I will try and make it as good as possible.

I will keep the master branch stable, and will push anything new to dev (as is customary).

# How to compile and install.
1. Clone the repo.
2. Open in Visual Studio (developed on VS2019, it will probably work on older versions, as long as C++17 is supported)
3. Set the configuration to Debug or Release, depending on what you want, and arch to x64. x64 is what the included libs are compiled for. If you want x86, recompile the libs yourself, replace them and set the config to x86.
4. Build the solution.
5. Copy the config.yaml file next to the newly build .exe and change it as needed.
6. From the redist directory, copy all the files next to the new .exe
7. Run the compressor.

# How to use/configure
You edit the values in the config.yaml file (which should be called that and sit next to the .exe, but you can specify a path as a CLI arugment). Most of them are self-explanatory, but I will focus on a few important ones.

encoderName: The name of the ffmpeg encoder you want to use. It supports most of them, but if it doesn't have it, recompile ffmpeg with the codecs you need, replace the files in the vendor/lib directory and recompile the program.

thread_count: This doesn't really do anything for encoders like NVENC, for others, it may have diffirent effects. Again, this is quite encoder dependent.

priv_data: This sets additional options for the encoder. This is very encoder dependent, so look them up for the encoder you use (ffmpeg -hide_banner -h encoder=[encoder name goes here]). Make sure to keep the spacing for the options the same, because it has to be parsed into a YAML map. Notepad++ handles the spacing automatically when you hit enter to go to a new line.

# Notes
NOTE: Currently the ndi-server is both the server and decompressor. It doesn't really resend anything to anyone. It just receves a signal and transforms it into a NDI source.

NOTE 2: This is, and always will be (or at least until NewTek release a cross-platform version of the SDK) Windows only, unlike the ndi-server, which eventually will be cross-platform.

# Updates
UPDATE 1: Multi-threading the various stages ended up being not a good idea, impact was negative in my testing. Currently the system is back to a single thread for video and another one for audio. I may revisit the idea at some point. Did some testing over WAN and network performance is currently a big problem. This is my main issue to solve. It works well over LAN (duh, so does NDI) as well as Wi-Fi as long as the bitrate is kept in check.
I also added support for the Optik profiler. By default it is enabled, if you want to disable it, remove the "#define __PROFILE" in Profiler.h and recompile.

# TODO
1. Build system support.
2. Build most of the dependencies with the build system, instead of including precompiled binaries (aside from NDI lib, which isn't open source)
3. Some sort of compression for audio. Currently thinking ZLib or LZ4, but could also be fed into ffmpeg.
