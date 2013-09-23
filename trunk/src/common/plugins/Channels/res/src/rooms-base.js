(function() {
  'use strict';

  schat.ui.MainWidget.prototype.toUrl = function(id, name, options) {
    return '<a href="' + ChannelsView.toUrl(id, name) + '" class="nick color-' + (options & 4 ? 'green' : 'default') + '" id="' + id + '">' + schat.utils.encode(name) + '</a>';
  };

  var navbar = new schat.ui.RoomsNavbar();
  var rooms  = new schat.ui.MainWidget();

  if (typeof ChannelsView === 'undefined') {
    window.ChannelsView = {
      join:   function(name)               {},
      create: function(id, name, _private) {},
      toUrl:  function(id, name)           { return '#'; }
    }
  }
  else {
    ChannelsView.feed.connect(function(json)      { feed(json) });
    ChannelsView.serverRenamed.connect(function() { navbar.setName.apply(navbar, arguments); });
  }


  if (typeof SimpleChat !== 'undefined') {
    SimpleChat.online.connect(function() { online(); });
  }


  /**
   * Успешное подключение к серверу.
   */
  function online() {
    var channel = SimpleChat.channel(SimpleChat.serverId());
    if (channel !== null)
      navbar.setName.call(navbar, channel.Name);

    SimpleChat.feed(SimpleChat.serverId(), 'list',  1);
    SimpleChat.feed(SimpleChat.serverId(), 'stats', 1);
  }


  /**
   * Чтение новых данных фидов.
   */
  function feed(json) {
    if (json.feed == 'list') {
      if (json.type == 'body' && (json.status == 200 || json.status == 303))
        rooms.setRooms(json.data.channels);
    }
  }


  function ready() {
    online();

    /**
     * Обработка нажатия на ссылку входа в канал по его идентификатору.
     */
    $('body').on('click.channel', '.nick', function (event) {
      event.preventDefault();
      ChannelsView.join($(this).attr('id'));
    });
  }

  $(document).ready(ready);
})();