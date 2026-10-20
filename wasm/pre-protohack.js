// Give this game its own IDBFS namespace so serving other ports at the
// same origin cannot overwrite their saves. Keep the native relative paths.
Module['preRun'] = Module['preRun'] || [];
Module['preRun'].push(function () {
  ENV['USER'] = ENV['LOGNAME'] = 'player';
  ENV['TERM'] = 'xterm';
});

// This runs after the template's initial IndexedDB load and asset preload,
// before main. Seed the score file once and route updates into IDBFS too.
var protohackRuntimeReady = Module['onRuntimeInitialized'];
Module['onRuntimeInitialized'] = function () {
  if (FS.analyzePath('/save').exists) FS.rmdir('/save');
  FS.symlink('/protohack-save', '/save');
  if (!FS.analyzePath('/protohack-save/record').exists)
    FS.writeFile('/protohack-save/record', FS.readFile('/record'));
  FS.unlink('/record');
  FS.symlink('/protohack-save/record', '/record');
  if (protohackRuntimeReady) protohackRuntimeReady();
};
