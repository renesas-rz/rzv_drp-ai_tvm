/*
 * websocket_demo.js
 *
 * Copyright (c) 2023 Renesas Electronics Corp.
 * This software is released under the MIT License,
 * see https://opensource.org/licenses/MIT
 */

const ID_Unknown   =  0;
const ID_DEEPPOSE   = 1;
const ID_YOLOV3     = 2;
const ID_TINYYOLOV3 = 3;
const ID_YOLOV2     = 4;
const ID_TINYYOLOV2 = 5;
const ID_ULTRAFACE  = 6;
const ID_HRNET      = 7;
const ID_HRNETV2    = 8;
const ID_GOOGLENET  = 9;
const ID_EMOTIONFP  = 10;

// let socket = new WebSocket('ws://localhost:3000/ws/', 'graph-update');
let socket = new WebSocket('ws://192.168.1.11:3000/ws/');
let predCanvas = document.getElementById('pred_canvas');
let defaultCanvas = document.getElementById('default_canvas');
let predCtx = document.getElementById('pred_canvas').getContext('2d');
let defaultCtx = document.getElementById('default_canvas').getContext('2d');
let graphCtx = document.getElementById('graph_canvas').getContext('2d');
let nowTimes = document.getElementsByClassName('now_time');
let predWindowData = document.getElementById('pred_window');
let drpWindowData = document.getElementById('drp_window');
let aiName = document.getElementById('ai_name');
let caution = document.getElementById('caution');
let aiDescription = document.getElementById('ai_description');

defaultCtx.fillStyle = 'darkgray';
defaultCtx.fillRect(0, 0, 640, 480);
defaultCtx.font = '12pt Arial';

predCtx.fillStyle = 'darkgray';
predCtx.fillRect(0, 0, 640, 480);
predCtx.font = '12pt Arial';

let model_id = ID_Unknown;

let startTime = null;

let postProcessTime = null;
let cam_postProcessTime = null;
let renderingTime = 0.0;

let endTime = moment();
let camera_endTime = moment();
let totalTime = 0.0;

let totalSpeed = 0.0;

let predTime = 0.0;
let camTime = 0.0;

let webcam = new Image();
let orgcam = new Image();

// Note: Comment out this chart, if you want to disable CPU usage graph
let graphChart = new Chart(graphCtx, {
  type: 'line',
  data: {
    labels: [],
    datasets: [
      {
        label: 'CPU 0 Usage[%]',
        data: [],
        borderColor: 'blue',
        backgroundColor: 'rgba(0,0,0,0)'
      },
      {
        label: 'CPU 1 Usage[%]',
        data: [],
        borderColor: 'deepskyblue',
        backgroundColor: 'rgba(0,0,0,0)'
      }
    ],
  },
  options: {
    scales: {
      y: {
        type: 'linear',
        min: 0,
        max: 100
      }
    },
    animation: {
      duration: 0
    }
  }
}
);

// Send Command: Model Change
let model = document.getElementById('change_model');
model.addEventListener('change', inputChange);

function inputChange(event) {
  // console.log(event.currentTarget.value);
  socket.send(JSON.stringify({
    command_name: 'change_model',
    Value: {
      model: event.currentTarget.value
    }
  }));
  if ((event.currentTarget.value == "TVM_DRPAI_DEEPPOSE") ||
      (event.currentTarget.value == "TVM_CPU_DEEPPOSE"))
  {
      aiName.innerHTML = "DeepPose: Facial Landmark Localization";
      caution.innerHTML= "<font>Please get close to the camera at around 20cm.";
      caution.style.color="#FFD700";
      aiDescription.innerHTML = "If your face is too far from the camera, the localization may fail.";
      aiDescription.style.color ="";
      model_id = ID_DEEPPOSE;
  }
  else if (event.currentTarget.value == "TVM_DRPAI_YOLOV3")
  {
      aiName.innerHTML = "YOLOv3: Object Detection";
      caution.innerHTML= "Detects 80 class of objects.";
      caution.style.color="";
      aiDescription.innerHTML = "</br>";
      aiDescription.style.color ="";
      model_id = ID_YOLOV3;
  }
  else if (event.currentTarget.value == "TVM_DRPAI_TINYYOLOV3")
  {
      aiName.innerHTML = "Tiny YOLOv3: Object Detection";
      caution.innerHTML= "Detect 80 class of objects.";
      caution.style.color="";
      aiDescription.innerHTML = "</br>";
      aiDescription.style.color ="";
      model_id = ID_TINYYOLOV3;
  }
  else if (event.currentTarget.value == "TVM_DRPAI_YOLOV2")
  {
      aiName.innerHTML = "YOLOv2: Object Detection";
      caution.innerHTML= "Detect 20 class of objects.";
      caution.style.color="";
      aiDescription.innerHTML = "</br>";
      aiDescription.style.color ="";
      model_id = ID_YOLOV2;
  }
  else if (event.currentTarget.value == "TVM_DRPAI_TINYYOLOV2")
  {
      aiName.innerHTML = "Tiny YOLOv2: Object Detection";
      caution.innerHTML= "Detect 20 class of objects.";
      caution.style.color="";
      aiDescription.innerHTML = "</br>";
      aiDescription.style.color ="";
      model_id = ID_TINYYOLOV2;
  }
  else if (event.currentTarget.value == "TVM_DRPAI_ULTRAFACE")
  {
      aiName.innerHTML = "UltraFace: Face Detection";
      caution.innerHTML= "Detect Human Faces.";
      caution.style.color="";
      aiDescription.innerHTML = "</br>";
      aiDescription.style.color ="";
      model_id = ID_ULTRAFACE;
  }
  else if (event.currentTarget.value == "TVM_DRPAI_HRNET")
  {
      aiName.innerHTML = "HRNet: Human Pose Estimation";
      caution.innerHTML= "<font>Please adjust the camera so that the whole body appears within the box.";
      caution.style.color="#FFD700";
      aiDescription.innerHTML = "Single person only. It does not support more than one person.</br>";
      aiDescription.style.color ="";
      model_id = ID_HRNET;
  }
  else if (event.currentTarget.value == "TVM_DRPAI_HRNETV2")
  {
      aiName.innerHTML = "HRNetV2: Hand Landmark Localization";
      caution.innerHTML= "<font>Please adjust the camera so that the hand above the wrist is inside the box.";
      caution.style.color="#FFD700";
      aiDescription.innerHTML = "Single hand only. It does not support more than one hand.</br>";
      aiDescription.style.color ="";
      model_id = ID_HRNETV2;
  }
  else if (event.currentTarget.value == "TVM_DRPAI_GOOGLENET")
  {
      aiName.innerHTML = "GoogleNet: Classification";
      caution.innerHTML= "Classify object in the frame.";
      caution.style.color="";
      aiDescription.innerHTML = "</br>";
      aiDescription.style.color ="";
      model_id = ID_HRNETV2;
  }
  else if (event.currentTarget.value == "TVM_DRPAI_EMOTIONFP")
  {
      aiName.innerHTML = "Emotion FERPlus: Emotion Recognition";
      caution.innerHTML= "Classify the human face expression. For face detection, UltraFace is used.";
      caution.style.color="";
      aiDescription.innerHTML ="<font>Pre-processing time includes entire Face Detection and Emotion FERPlus pre-processing. All processing time is cummurative value for detected boxes.</br>";
      aiDescription.style.color ="#FFD700";
      model_id = ID_EMOTIONFP;
  }
}

// Pose Estimation: Line Drawing
function drawLine(ctx, pts, start, end, ratio_x, ratio_y) {
  if ((pts[start] === null) || (pts[end] === null)) {
    return;
  }
  else if ((pts[start] === undefined) || (pts[end] === undefined)) {
    return;
  }
  else if ((pts[start] === 0) && pts[end] === 0) {
    return;
  }
  else {
    ctx.moveTo((pts[start].X) * ratio_x, (pts[start].Y) * ratio_y); // start point
    ctx.lineTo((pts[end].X) * ratio_x, (pts[end].Y) * ratio_y); // end point
    ctx.stroke(); // drawing
    return;
  }
}

// Pose Estimation: Point Drawing
function drawKeyPoint(ctx, pts, ratio_x, ratio_y) {
  ctx.beginPath();
  ctx.arc((pts.X) * ratio_x, (pts.Y) * ratio_y, 3, 0, (2 * Math.PI));
  ctx.closePath();
  ctx.fill();
}

// USB Camera Image Process Time Measurement
function measureCameraTime(ctx, nowTime) {
  cam_postProcessTime = moment();
  camTime = (1000 / cam_postProcessTime.diff(camera_endTime)).toFixed(3);
  ctx.fillText('FPS: ' + camTime, 540, 20);
  camera_endTime = moment();
}

// Process Time Measurement
function measureProcessingTime(ctx, nowTime) {
  // console.log('----------------------------------------');
  postProcessTime = moment();
  // console.log('postProcessTime ' + postProcessTime);

  // Time from JSON receive -> Finish drawing
  renderingTime = postProcessTime.diff(nowTime);
  // Total Time
  totalTime = postProcessTime.diff(endTime);
  // Process Speed
  totalSpeed = (1000 / totalTime);

  ctx.fillText('Rendering time: ' + renderingTime + ' ms', 470, 20);
  // ctx.fillText('Total time:     ' + totalTime + ' ms', 470, 40);
  ctx.fillText('FPS:                    ' + totalSpeed.toFixed(2), 470, 40);

  // console.log('renderingTime ' + renderingTime);
  // console.log('totalTime ' + totalTime);
  // console.log('totalSpeed ' + totalSpeed);

  endTime = moment();
  // console.log('endTime ' + endTime);
  // console.log('----------------------------------------');
}

/* application message */
let disp_application_message = null;

let is_dialog_shown= false;
$('#dialog').on('show.bs.modal', function (e) {
  is_dialog_shown=true;
  console.log('shown');
  //document.getElementById('dialog-text').innerHTML = disp_application_message.join("<hr>");
  if(disp_application_message!=null)
  {
    document.getElementById('dialog-text').innerText = disp_application_message;
  }
});
$('#dialog').on('hidden.bs.modal', function (e) {
  is_dialog_shown=false;
  console.log('hidden');
  //disp_application_message.splice(0);
  disp_application_message=null;
});

$(() => {
  socket.onmessage = function (event) {
    // Calculate process time
    let nowTime = moment();
    if (startTime === null) {
      startTime = moment();
    }
    let cnt = nowTime.diff(startTime, 'seconds');
    let elapsedTime = moment('2022-04-01 00:00:00').add(cnt, 's').format('HH:mm:ss');
    nowTimes[0].innerHTML = elapsedTime;
    nowTimes[1].innerHTML = elapsedTime;

    // JSON parse
    let datas = JSON.parse(event.data);
    console.log(datas);

    let predData;
    let predDatas = [];
    let predStr;
    let drpData = null;
    let len;
    let i = 0;

    // Update CPU Usage Graph
    if (datas.command_name === 'cpu_usage') {
      graphChart.data.labels.push(nowTime.format('HH:mm:ss'));
      let cpuUsages = datas.Value.cpu_usage.split(' ');
      graphChart.data.datasets[0].data.push(cpuUsages[0]);
      graphChart.data.datasets[1].data.push(cpuUsages[1]);

      if (graphChart.data.labels.length > 10) {
        graphChart.data.labels.shift();
        graphChart.data.datasets.forEach((dataset) => {
          dataset.data.shift();
        });
      }
      graphChart.update();
    }
    else if (datas.command_name === 'camera_image') {
      orgcam.src = 'data:image/jpeg;base64,' + datas.Value.img;

      orgcam.onload = function () {
        if (datas.Value.img) {
          defaultCtx.drawImage(orgcam, 0, 0, 640, 480);

          measureCameraTime(defaultCtx, nowTime);
        }
      }
    }
    // YOLOv3, TinyYOLOv2
    else if (datas.command_name === 'object_detection') {
      predCtx.linewidth = 8;
      predCtx.strokeStyle = 'blue';
      predCtx.fillStyle = 'blue';
      defaultCtx.fillStyle = 'blue';
      if ((model_id == ID_ULTRAFACE) || (model_id == ID_EMOTIONFP))
      {
        predCtx.strokeStyle = 'yellow';
        predCtx.fillStyle = 'yellow';
        defaultCtx.fillStyle = 'yellow';
      }
      predData = datas.Value.predict;
      len = predData.length;

      let cls = [];
      let pred = [];
      let x = [];
      let y = [];
      let w = [];
      let h = [];

      for (i = 0; i < len; i++) {
        predStr = predData[i];
        cls[i] = String(predStr.name);
        pred[i] = Number.parseFloat(predStr.pred).toFixed(2);
        x[i] = Number(predStr.X);
        y[i] = Number(predStr.Y);
        w[i] = Number(predStr.W);
        h[i] = Number(predStr.H);

        if (i !== 0) {
            if (model_id == ID_ULTRAFACE )
            {
                predDatas[i] = '\n' + pred[i] + ' %\t' +
                    'X: ' + (x[i] + "   ").slice(0, 4)+
                    '\tY: ' +  (y[i] + "   ").slice(0, 4) +
                    '\tW: ' +  (w[i] + "   ").slice(0, 4) +
                    '\tH: ' +  (h[i] + "   ").slice(0, 4);
            }
            else
            {
                predDatas[i] = '\n' + cls[i] + ' :\t' + pred[i] + ' %\t' + 'X: ' + x[i] + '\tY: ' + y[i] + '\tW: ' + w[i] + '\tH: ' + h[i];
            }
        }
        else {
            if (model_id == ID_ULTRAFACE )
            {
                predDatas[i] = pred[i] + ' %\t' +
                    'X: ' + (x[i] + "   ").slice(0, 4)+
                    '\tY: ' +  (y[i] + "   ").slice(0, 4) +
                    '\tW: ' +  (w[i] + "   ").slice(0, 4) +
                    '\tH: ' +  (h[i] + "   ").slice(0, 4);
            }
            else
            {
                predDatas[i] = cls[i] + ' :\t' + pred[i] + ' %\t' + 'X: ' + x[i] + '\tY: ' + y[i] + '\tW: ' + w[i] + '\tH: ' + h[i];
            }
        }
      }

      webcam.src = 'data:image/jpeg;base64,' + datas.Value.img;


      let ratio_w = predCanvas.width / datas.Value.img_org_w;
      let ratio_h = predCanvas.height / datas.Value.img_org_h;
      console.log('ratio_w ' + ratio_w);
      console.log('ratio_h ' + ratio_h);



      webcam.onload = function () {
        // Output USB camera image
        predCtx.drawImage(webcam, 0, 0, predCanvas.width, predCanvas.height);

        for (i = 0; i < len; i++) {
          predCtx.strokeRect(x[i] * ratio_w, y[i] * ratio_h, w[i] * ratio_w, h[i] * ratio_h);
          predCtx.fillText(cls[i], x[i] * ratio_w, (y[i] * ratio_h + h[i] * ratio_h + 16));
        }

        measureProcessingTime(predCtx, nowTime);
      }

      // Calculate & Display process time
      drpData = 'Inference time:' + '\t' + Number.parseFloat(datas.Value.drp_time).toFixed(2) + ' ms\n' +
                'Pre-process time:' + '\t' + Number.parseFloat(datas.Value.pre_time).toFixed(2) + ' ms\n' +
                'Post-process time:' + '\t' + Number.parseFloat(datas.Value.post_time).toFixed(2) + ' ms\n';

      predWindowData.value = predDatas;
      drpWindowData.value = drpData;
    }
    // HRNet
    else if (datas.command_name === 'pose_detection') {
      predCtx.linewidth = 8;
      predCtx.strokeStyle = "yellow";
      predCtx.fillStyle = 'yellow';
      defaultCtx.fillStyle = "yellow";

      predData = datas.Value.predict;
      len = predData.length;

      for (i = 0; i < len; i++) {
        if (i !== 0) {
          predDatas[i] = '\n' + 'No.' + (i + 1) + '\t' + 'X: ' + predData[i].X + '\tY: ' + predData[i].Y;
        }
        else {
          predDatas[i] = 'No.' + (i + 1) + '\t' + 'X: ' + predData[i].X + '\tY: ' + predData[i].Y;
        }
      }


      webcam.src = 'data:image/jpeg;base64,' + datas.Value.img;
      let ratio_w = predCanvas.width / datas.Value.img_org_w;
      let ratio_h = predCanvas.height / datas.Value.img_org_h;

      webcam.onload = function () {
        // Display Image
        predCtx.drawImage(webcam, 0, 0, predCanvas.width, predCanvas.height);


        if (model_id == ID_DEEPPOSE) {
          // Draw Inference Crop Range (VGA: X = 185/ Y = 0/ Width = 270/ Height = 480)
          predCtx.strokeRect(80 * ratio_w, 2 * ratio_h, 480 * ratio_w, 476 * ratio_h);
          predCtx.fillText("Please fit your face into this yellow box.", (80 + 5) * ratio_w, (datas.Value.img_org_h - 5) * ratio_h);

          console.log('ratio_w ' + ratio_w);
          console.log('ratio_h ' + ratio_h);


          // Draw keypoint and display inference info
          for (i = 0; i < len; i++) {
            drawKeyPoint(predCtx, predData[i], ratio_w, ratio_h);
          }
        }
        else if (model_id == ID_HRNETV2) {
	        // Draw Inference Crop Range (VGA: X = 80/ Y = 0/ Width = 480/ Height = 480)
	        predCtx.strokeRect(80 * ratio_w, 0 * ratio_h, 480 * ratio_w, 480 * ratio_h);
	        predCtx.fillText("Please align this position", (80 + 5) * ratio_w, (datas.Value.img_org_h - 5) * ratio_h);

	        // Draw hand
          predCtx.beginPath();
          predCtx.strokeStyle = 'aqua';
	        drawLine(predCtx, predData, 0, 1, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 1, 2, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 2, 3, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 3, 4, ratio_w, ratio_h);
          predCtx.beginPath();
          predCtx.strokeStyle = 'fuchsia';
	        drawLine(predCtx, predData, 0, 5, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 5, 6, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 6, 7, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 7, 8, ratio_w, ratio_h);
          predCtx.beginPath();
          predCtx.strokeStyle = 'yellow';
	        drawLine(predCtx, predData, 0, 9, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 9, 10, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 10, 11, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 11, 12, ratio_w, ratio_h);
          predCtx.beginPath();
          predCtx.strokeStyle = 'blue';
	        drawLine(predCtx, predData,  0, 13, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 13, 14, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 14, 15, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 15, 16, ratio_w, ratio_h);
          predCtx.beginPath();
          predCtx.strokeStyle = 'lime';
	        drawLine(predCtx, predData,  0, 17, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 17, 18, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 18, 19, ratio_w, ratio_h);
          drawLine(predCtx, predData, 19, 20, ratio_w, ratio_h);

	        console.log('ratio_w ' + ratio_w);
	        console.log('ratio_h ' + ratio_h);

	        // Draw keypoint and display inference info
	        for (i = 0; i < len; i++) {
	          drawKeyPoint(predCtx, predData[i], ratio_w, ratio_h);
	        }
        }
        else if (model_id == ID_HRNET) {
	        // Draw Inference Crop Range (VGA: X = 184/ Y = 0/ Width = 270/ Height = 480)
	        predCtx.strokeRect(184 * ratio_w, 0 * ratio_h, 270 * ratio_w, 480 * ratio_h);
	        predCtx.fillText("Please stand here", (185 + 5) * ratio_w, (datas.Value.img_org_h - 5) * ratio_h);

	        // Draw Skeleton
	        drawLine(predCtx, predData, 0, 1, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 1, 2, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 2, 0, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 1, 3, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 2, 4, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 3, 5, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 4, 6, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 5, 6, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 5, 7, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 6, 8, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 7, 9, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 8, 10, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 5, 11, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 6, 12, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 11, 12, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 11, 13, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 12, 14, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 13, 15, ratio_w, ratio_h);
	        drawLine(predCtx, predData, 14, 16, ratio_w, ratio_h);

	        console.log('ratio_w ' + ratio_w);
	        console.log('ratio_h ' + ratio_h);


	        // Draw keypoint and display inference info
	        for (i = 0; i < len; i++) {
	          drawKeyPoint(predCtx, predData[i], ratio_w, ratio_h);
	        }
        }

        // Measure prcess time
        measureProcessingTime(predCtx, nowTime);
      }

      // Calculate & Display process time
      drpData = 'Inference time:' + '\t' + Number.parseFloat(datas.Value.drp_time).toFixed(2) + ' ms\n' +
                'Pre-process time:' + '\t' + Number.parseFloat(datas.Value.pre_time).toFixed(2) + ' ms\n' +
                'Post-process time:' + '\t' + Number.parseFloat(datas.Value.post_time).toFixed(2) + ' ms\n';


      predWindowData.value = predDatas;
      drpWindowData.value = drpData;
    }
    // GoogleNet
    else if (datas.command_name === 'classification') {
      predCtx.linewidth = 8;
      predCtx.strokeStyle = 'red';
      predCtx.fillStyle = 'red';
      defaultCtx.fillStyle = 'red';

      predData = datas.Value.predict;
      len = predData.length;

      let cls = [];
      let pred = [];

      for (i = 0; i < len; i++) {
        predStr = predData[i];
        cls[i] = String(predStr.names);
        pred[i] = Number.parseFloat(predStr.pred).toFixed(2);

        if (i !== 0) {
          predDatas[i] = '\n' + pred[i] + '% :\t' + cls[i] + ' ';
        }
        else {
          predDatas[i] = pred[i] + '% :\t' + cls[i];
        }
      }

      webcam.src = 'data:image/jpeg;base64,' + datas.Value.img;
      webcam.onload = function () {
        // Dispaly USB camera image
        predCtx.drawImage(webcam, 0, 0, 640, 480);

        // Display inference result
        for (i = 0; i < len; i++) {
          predCtx.fillText(cls[i], 5, 20 + (20 * i));
        }

        // Calculate process time
        measureProcessingTime(predCtx, nowTime);
      }

      // Calculate & Display process time
      drpData = 'Inference time:' + '\t' + Number.parseFloat(datas.Value.drp_time).toFixed(2) + ' ms\n' +
                'Pre-process time:' + '\t' + Number.parseFloat(datas.Value.pre_time).toFixed(2) + ' ms\n' +
                'Post-process time:' + '\t' + Number.parseFloat(datas.Value.post_time).toFixed(2) + ' ms\n';

      predWindowData.value = predDatas;
      drpWindowData.value = drpData;
    }
    else if(datas.command_name === 'app_message')
    {
          console.debug(datas.Value.message);
          if(disp_application_message ==null)
          {
            disp_application_message = datas.Value.message;
          }
          if(is_dialog_shown ==false)
          {
            $('#dialog').modal({
              backdrop: 'static'
            });
          }
    }
  }
})
