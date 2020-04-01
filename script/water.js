
/*createState("javascript.0.monitor.DayHotWater","");
createState("javascript.0.monitor.MonthHotWater","");
createState("javascript.0.monitor.DayHotRate","");
createState("javascript.0.monitor.MonthHotRate","");
createState("javascript.0.monitor.DayColdWater","");
createState("javascript.0.monitor.MonthColdWater","");
createState("javascript.0.monitor.DayColdRate","");
createState("javascript.0.monitor.MonthColdRate","");
*/

sendTo('pushover.0', 'Скрипт Расчета расхода Воды запущен');

schedule('{"time":{"start":"23:45","end":"23:59","mode":"minutes","interval":5},"period":{"days":1}}', function () {

  var date = new Date();

  //var  w_day = date.getDay(); // 0-воскр. 6-суббота

  var hour = date.getHours();

  var day = date.getDate();

  var min = date.getMinutes();

  var DayHotWater = getState('mqtt.0.home.watermeter.Hot:__'/*home/sensors/watercount/Hot:*/).val;
  var DayColdWater = getState('mqtt.0.home.watermeter.Cold:_'/*home/sensors/watercount/Cold:*/).val;
    log('Текущее значение счетчика Холодной воды' +DayColdWater);
    log('Текущее значение счетчика Горячей воды' +DayHotWater);
   
   if(hour == 23 && min == 55) { /*каждый день в 23-55 сохраняем данные энерго потребления */

    var last_DayHotWater = getState("javascript.0.monitor.DayHotWater").val;   
    var last_DayColdWater = getState("javascript.0.monitor.DayColdWater").val; 
    var rate1= DayHotWater -last_DayHotWater;
    var rate2= DayColdWater -last_DayColdWater;

    setState ('javascript.0.monitor.DayHotWater'/*javascript 0 monitor DayHotWater*/, DayHotWater.toFixed(2));
    setState ('javascript.0.monitor.DayColdWater'/*javascript 0 monitor DayColdWater*/, DayColdWater.toFixed(2));
    setState ('javascript.0.monitor.DayHotRate'/*javascript 0 monitor DayHotRate*/, rate1.toFixed(2));
    setState ('javascript.0.monitor.DayColdRate'/*javascript 0 monitor DayHotRate*/, rate2.toFixed(2));

    log('Сохранено дневное потребление Горячей воды ' +rate1.toFixed(2));
    log('Сохранено дневное потребление Холодной воды '+rate2.toFixed(2));

    sendTo('pushover.0', 'Сохранено дневное потребление Холодной воды: ' +rate2.toFixed(2) +' at '+hour +':'
    +min );
        sendTo('pushover.0', 'Сохранено дневное потребление Горячей воды: ' +rate1.toFixed(2) + ' at '+hour +':'
    +min );
   }

  if(day == 10 && hour == 23 && min == 55) { /*10 числа каждого месяца сохраняем данные энерго потребления */

    var last_MonthHotWater = getState("javascript.0.monitor.MonthHotWater").val;   
    var last_MonthColdWater = getState("javascript.0.monitor.MonthColdWater").val; 
    var Month_rate1= DayHotWater -last_MonthHotWater;
    var Month_rate2= DayColdWater -last_MonthColdWater;

    setState ('javascript.0.monitor.MonthHotWater', DayHotWater.toFixed(2));
    setState ('javascript.0.monitor.MonthColdtWater', DayColdWater.toFixed(2));
    setState ('javascript.0.monitor.MonthHotRate',Month_rate1.toFixed(2));
    setState ('javascript.0.monitor.MonthColdRate', Month_rate2.toFixed(2));

    log('Сохранено месячное потребление Горячей воды '+Month_rate1.toFixed(2));
    log('Сохранено месячное потребление Холодной воды '+Month_rate2.toFixed(2));
    sendTo('pushover.0', 'Сохранено месячное потребление Холодной воды: '+Month_rate2.toFixed(2) +' at '+hour +':'
    +min );
        sendTo('pushover.0', 'Сохранено месячное потребление Горячей воды: '+Month_rate1.toFixed(2) +' at '+hour +':'
    +min );
}

});