   // Load the AWS SDK
   var AWS = require("aws-sdk");
   // Set up the code to call when the Lambda function is invoked
   exports.handler = (event, context, callback) => {
      
   // thresholds
   
   // Sensor Datasheet

   //(10 G) Stunt Safety Limit
   var accel = 100 
   // WHO NOM Limits kPa
   var maxP = 82 
   var minP = 74 
   // Max 45 Degrees 
   var maxD = 45 
   // Normal sensor values on a flat surface
   var xnom=-3
   var ynom = -8
   var znom = 98
   // WHO max µTeslas
   var maxMag = 100 
   // dartmouth college standard
   var maxTemp = 30 
   var minTemp = 15
   var maxH = 80
   var minH = 20
   
   // Function
   
   var eventText = JSON.parse(JSON.stringify(event, null, 2));
   var sensor;
   var value;

      if (eventText["Acc"] != undefined) {
      sensor="Accel"
      var x = Math.abs(parseInt(eventText["Acc"].split(",")[0])*0.1);
      var y = Math.abs(parseInt(eventText["Acc"].split(",")[1])*0.1);
      var z = Math.abs(parseInt(eventText["Acc"].split(",")[2])*0.1);
      value=eventText["Acc"]
      if(x > accel  || y > accel || z > accel )
      {
         var sns = new AWS.SNS();
         var params = {
         Message: "Excessive movement detected",
         TopicArn: "arn:aws:sns:us-east-1:xxxxxxxxxxxxx:YOURTOPIC"
         };
         sns.publish(params, context.done);
      }
   }
   else if (eventText["Gyro"] != undefined) {
      sensor="Gyro"
      var x = parseInt(eventText["Gyro"].split(",")[0])*0.1;
      var y = parseInt(eventText["Gyro"].split(",")[1])*0.1;
      var z = parseInt(eventText["Gyro"].split(",")[2])*0.1;
      value=eventText["Gyro"]
       if(Math.abs(x-xnom) > maxMag  || Math.abs(y-ynom) > maxMag || Math.abs(z-znom) > maxMag )
      {
         var sns = new AWS.SNS();
         var params = {
         Message: "Strange position detected",
         TopicArn: "arn:aws:sns:us-east-1:xxxxxxxxxxxxx:YOURTOPIC"
         };
         sns.publish(params, context.done);
      }
   }
   else if (eventText["Mag"] != undefined) {
      sensor="Magnet"
      var  x = Math.abs(parseInt(eventText["Mag"].split(",")[0])*0.1);
      var y = Math.abs(parseInt(eventText["Mag"].split(",")[1])*0.1);
      var z = Math.abs(parseInt(eventText["Mag"].split(",")[2])*0.1);
      value=eventText["Mag"]
      if(x > maxMag  || y > maxMag || z > maxMag )
      {
         var sns = new AWS.SNS();
         var params = {
         Message: "High magnetic field detected.",
         TopicArn: "arn:aws:sns:us-east-1:xxxxxxxxxxxxx:YOURTOPIC"
         };
         sns.publish(params, context.done);
      }
   }
   else if (eventText["Temp"] != undefined) {
      sensor="Temp"
      value=parseInt(eventText["Temp"])
      
     if(value > maxH)
      {
         var sns = new AWS.SNS();
         var params = {
         Message: "High Temperature Detected",
         TopicArn: "arn:aws:sns:us-east-1:xxxxxxxxxxxxx:YOURTOPIC"
         };
         sns.publish(params, context.done);
      }
      else if(value < minH)
      {
         var sns = new AWS.SNS();
         var params = {
         Message: "Low Temperature Detected",
         TopicArn: "arn:aws:sns:us-east-1:xxxxxxxxxxxxx:YOURTOPIC"
         };
         sns.publish(params, context.done);
      }
      
   }
   else if (eventText["Press"] != undefined) {
      sensor="Press"
      value=parseInt(eventText["Press"])
      if(value > maxP)
      {
         var sns = new AWS.SNS();
         var params = {
         Message: "High Pressure Detected",
         TopicArn: "arn:aws:sns:us-east-1:xxxxxxxxxxxxx:YOURTOPIC"
         };
         sns.publish(params, context.done);
      }
      else if(value < minP)
      {
         var sns = new AWS.SNS();
         var params = {
         Message: "Low Pressure Detected",
         TopicArn: "arn:aws:sns:us-east-1:xxxxxxxxxxxxx:YOURTOPIC"
         };
         sns.publish(params, context.done);
      }
   }
   else if (eventText["Hum"] != undefined) {
      sensor="Hum"
      value=parseInt(eventText["Hum"])
      if(value > maxH)
      {
         var sns = new AWS.SNS();
         var params = {
         Message: "High Humidity Detected",
         TopicArn: "arn:aws:sns:us-east-1:xxxxxxxxxxxxx:YOURTOPIC"
         };
         sns.publish(params, context.done);
      }
      else if(value < minH)
      {
         var sns = new AWS.SNS();
         var params = {
         Message: "Low Humidity Detected",
         TopicArn: "arn:aws:sns:us-east-1:xxxxxxxxxxxxx:YOURTOPIC"
         };
         sns.publish(params, context.done);
      }
   }
   else{
      // Nothing
   }
   return [sensor,x]
    
};