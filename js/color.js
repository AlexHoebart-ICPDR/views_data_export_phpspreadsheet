(function ($, Drupal) {
  Drupal.behaviors.color = {
    attach: function (context, settings) {
      $(".color").attr("type","color");
    }
  };
})(jQuery, Drupal);

