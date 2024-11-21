import cv2                       # OpenCV library for computer vision operations
from picamera2 import Picamera2  # Library for interacting with Raspberry Pi's Picamera2
from ultralytics import YOLO     # YOLO (You Only Look Once) object detection model
import time                      # Library for time-related functions
import paho.mqtt.client as mqtt  # MQTT client for messaging
import json                      # Library for handling JSON data

# Callback executed when the client connects to the MQTT broker
def on_connect(client, userdata, flags, reason_code, properties):
    print(f"Connected with result code {reason_code}")
    # Automatically resubscribe to topics on reconnection
    client.subscribe("$SYS/#")

# Callback executed when a message is received from the MQTT broker
def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload))

# MQTT configuration: 
#   replace with the IP address or hostname of your MQTT broker
MQTT_SERVER = '127.0.0.1'  # MQTT broker address
MQTT_TOPIC = "/ai-vet/yolo/person"  # Topic to publish person detection state
lastPerson = False  # Variable to track the last detection state

# Initializes MQTT client and configures callbacks
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)  # Create an MQTT client instance
client.on_connect = on_connect  # Assign the on_connect callback
client.on_message = on_message  # Assign the on_message callback

client.connect(MQTT_SERVER, 1883, 60)  # Connect to the MQTT broker on port 1883
client.loop_start()  # Start the MQTT client loop to handle messages

# Initialize the Picamera2 for capturing video
picam2 = Picamera2()  # Create a Picamera2 instance
picam2.preview_configuration.main.size = (1280, 720)  # Set resolution
picam2.preview_configuration.main.format = "RGB888"  # Set color format
picam2.preview_configuration.align()  # Align configuration
picam2.configure("preview")  # Apply preview configuration
picam2.start()  # Start the camera

# Load and configure the YOLO model
model = YOLO("yolo11n.pt")  # Load a pre-trained YOLO model
model.export(format="ncnn")  # Export the model to NCNN format for compatibility
model = YOLO("yolo11n_ncnn_model")  # Reload the exported model

# Infinite loop for real-time object detection
while True:
    personNow = False  # Reset detection state for the current frame

    frame = picam2.capture_array() # Capture a frame from the camera

    results = model(frame) # Run YOLO model inference on the captured frame
    
    # Process detection results
    if results[0]:  # If there are detected objects
        label = results[0].names[int(results[0].boxes[0].cls[0])]  # Extract label of the first detected object
        confidence = results[0].boxes[0].conf[0].item()  # Extract confidence score
        # Check if the detected object is a person with high confidence
        if label == "person" and confidence > 0.90:
            print("Object:", label, "Confidence: ", confidence)
            personNow = True  # Update detection state
    
    # Publish detection state to the MQTT topic if it changes
    if personNow != lastPerson:
        client.publish(MQTT_TOPIC, int(personNow))  # Convert boolean to integer (1 or 0)
        lastPerson = personNow  # Update the last detection state

    # Break the loop if 'q' is pressed
    if cv2.waitKey(1) == ord("q"):
        break

    # Delay for 50ms to throttle processing rate
    time.sleep(0.050)

# Cleanup resources before exiting
client.loop_stop()  # Stop the MQTT client loop
client.disconnect()  # Disconnect from the MQTT broker
