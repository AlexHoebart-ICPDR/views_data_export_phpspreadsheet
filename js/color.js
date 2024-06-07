(function ($, Backdrop) {
  Backdrop.behaviors.color = {
    attach: function (context, settings) {
      $(".color").attr("type","color");
    }
  };
})(jQuery, Backdrop);

