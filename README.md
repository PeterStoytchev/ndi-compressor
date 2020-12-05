# ndi-compressor
Compresses a NDI source and outputs it to a ndi-server. Effectevly, this is a hacky implementation for NDI-HX, but it most certanly won't be as good compared to a hardware encoder, or if NewTek decide to release an actual NDI-HX software encoder (PLEASE DO IT).

You can think of the whole system, as an adapter, that sits in-between an NDI source and viewer. It captures the signal from the NDI source, compresses it (using ffmpeg libraries), sends it across the network (sockpp) to a server which will distribute it to ndi-compressor(s) which decompress it and turns it back into a NDI source which viewers can view. This enables NDI over low bandwidth networks, such as Wi-Fi or WAN, without the need for NDI-HX hardware or services which, essentailly, do the same thing (compress the signal to reduce bandwith).

I will keep the master branch stable, and will push anything new to dev (as is customary).

NOTE: Currently the ndi-server is both a server and decompressor. It doesn't really resend anything to anyone. It just receves a signal and transforms it into a NDI source.

NOTE 2: This is, and always will be (or at least until NewTek release a cross-platform version of the SDK) Windows only, unlike the ndi-server, which eventually will be cross-platform.

# How to compile and install.
1. Clone the repo.
2. Open in Visual Studio (developed on VS2019, it will probably work on older versions, as long as C++17 is supported)
3. Set the configuration to Debug or Release, depending on what you want, and arch to x86. Currently, only x86 is supported.
4. Build the solution.
5. Copy the config.yaml file next to the newly build .exe and change it as needed.
6. From the redist directory, copy all the files next to the new .exe
7. Run the compressor.

# How to use/configure
You edit the values in the config.yaml file (which should be called that and sit next to the .exe, but you can specify a path as a CLI arugment). Most of them are self-explenirary, but I will focus on a few important ones.

encoderName: The name of the ffmpeg encoder you want to use. It supports most of them, but if it doesn't have it, recompile ffmpeg with the codecs you need, replace the files in the vendor/lib directory and recompile the program.

thread_count: This doesn't really do anything for encoders like NVENC, for others, it may have diffirent effects. Again, this is quite encoder dependent.

priv_data: This sets additional options for the encoder. This is very encoder dependent, so look them up for the encoder you use (ffmpeg -hide_banner -h encoder=<encoder name goes here>). Make sure to keep the spacing for the options the same, because it has to be parsed into a YAML map. Notepad++ handles the spacing automatically when you hit enter to go to a new line.

# TODO
1. Profiling support.
2. Seperate the video processing stages (get a frame from NDI, compress and send to a ndi-server) onto diffirent threads, in order to reduce latency. 
3. Cmake support.
4. Move to x64.
5. Build most of the dependencies with the build system, instead of including precompiled binaries (aside from NDI lib, which isn't open source).
