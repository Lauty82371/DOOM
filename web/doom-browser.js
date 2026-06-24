mergeInto(LibraryManager.library, {
  js_console_log__sig: 'vi',
  js_console_log: function(ptr) {
    console.log(UTF8ToString(ptr));
  }
});
