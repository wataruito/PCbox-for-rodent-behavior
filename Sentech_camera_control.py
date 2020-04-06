"""
Sentech_camera_control.py
    Wataru Ito, 2020/03/22
    Modified from "derricw/pysentech/pysentech/examples/low_level_cv2.py"
Dependency:
    StCamUSBPack_EN_190207.zip (https://sentech.co.jp/en/products/USB/software.html)
Test environment:
    Sentech STC-TB33USB-AS, GPIO IO0 receives triggers
Install:
    1. Download Sentech driver and SDK
        StCamUSBPack_EN_190207.zip from https://sentech.co.jp/en/products/USB/software.html
        Unzip and rename "StCamUSBPack_EN_190207\3_SDK\TriggerSDK(v3.11)\bin\x64\StTrgApi.dll" to "StCamD.dll"
    2. Install the camera driver
    3. Set up python
        > conda create -n pysentech anaconda
        > pip install pysentech
        > pip install opencv-contrib-python
        > pip install pyserial
"""
import os
import traceback
import ctypes
from ctypes import *
# # malloc = ctypes.cdll.msvcrt.malloc  #windows
# # free = ctypes.cdll.msvcrt.free
import numpy as np
import cv2
from pysentech import SentechDLL
import threading

###################################
# load the dll
sdk_folder = r"C:\Users\User\Desktop\Sentech_camera\StCamUSBPack_EN_190207\3_SDK\TriggerSDK(v3.11)"
dll = SentechDLL(sdk_folder)
print("DLL loaded!")

###################################
# Entry
def live_movie(cameraNum, fps, savePath):
    ###################################
    # 1) Open a camera
    ###################################
#     _cameraNum = 0;
#     for _cameraNum in range(0, cameraNum):
#         hCamera[_cameraNum] = dll.StTrg_Open(0)
        
    hCamera = np.array([dll.StTrg_Open(0), dll.StTrg_Open(0), dll.StTrg_Open(0)])
    previewName = np.array(["camera_1","camera_2","camera_3"])
    
    if cameraNum > 0:
        thread1 = camThread(previewName[0], hCamera[0], fps, savePath)
        thread1.start()
    if cameraNum > 1:
        thread2 = camThread(previewName[1], hCamera[1], fps, savePath)
        thread2.start()
    if cameraNum > 2:
        thread3 = camThread(previewName[2], hCamera[2], fps, savePath)
        thread3.start()        
    
###################################
# Thread definition
class camThread(threading.Thread):
    def __init__(self, previewName, camID, fps, savePath):
        threading.Thread.__init__(self)
        self.previewName = previewName
        self.camID = camID
        self.fps = fps
        self.savePath = savePath  
    def run(self):
        print ("Starting " + self.previewName)
        acquire_images(self.previewName, self.camID, self.fps, self.savePath )

###################################
# Thread detailes    
def acquire_images(previewName, camera, fps, savePath):
  
    ###################################
    # 2) Get image shape
    ###################################
    # Get ScanMode
    wScanMode = c_ushort()  # WORD  wScanMode;
    dwOffsetX = c_ulong()   # DWORD dwOffsetX;
    dwOffsetY = c_ulong()   # DWORD dwOffsetY;
    dwWidth = c_ulong()     # DWORD dwWidth;
    dwHeight = c_ulong()    # DWORD dwHeight;
    if not dll.StTrg_GetScanMode(camera, 
                                 byref(wScanMode), byref(dwOffsetX), byref(dwOffsetY),
                                 byref(dwWidth), byref(dwHeight)):
        print("Couldn't get ScanMode.")

    width, height = dwWidth.value, dwHeight.value
    print("Camera image shape: {}x{}".format(width, height))

    ###################################
    # 3) Pixel format
    ################################### 

    ################################### 
    # 4) Get TransferBitsPerPixel
    ###################################
    # Get TransferBitsPerPixel
    dwTransferBitsPerPixel = c_ulong() # DWORD  dwTransferBitsPerPixel;
    if not dll.StTrg_GetTransferBitsPerPixel(camera, byref(dwTransferBitsPerPixel)):
        print("Couldn't get TransferBitsPerPixel.")

    if dwTransferBitsPerPixel.value == dll.STCAM_TRANSFER_BITS_PER_PIXEL_RAW_08:
        pass
    elif dwTransferBitsPerPixel.value == dll.STCAM_TRANSFER_BITS_PER_PIXEL_RAW_10:
        pass
    elif dwTransferBitsPerPixel.value == dll.STCAM_TRANSFER_BITS_PER_PIXEL_RAW_12:
        pass
    else:
        print("Not expected TransferBitsPerPixel value.")

    ###################################
    # 5) Get bytes per image
    ###################################
    bpi = width * height

    ###################################
    # 6) Trigger setting
    ###################################
    dwTriggerMode = c_ulong(dll.STCAM_TRIGGER_MODE_TYPE_TRIGGER)
    dwTriggerMode = c_ulong(dll.STCAM_TRIGGER_MODE_TYPE_TRIGGER + dll.STCAM_TRIGGER_MODE_EXPTIME_PULSE_WIDTH
                           + dll.STCAM_TRIGGER_MODE_SOURCE_HARDWARE +dll.STCAM_TRIGGER_MODE_EXPOSURE_WAIT_HD_ON)
    dll.StTrg_SetTriggerMode(camera,dwTriggerMode)

    ###################################
    # 7) IO pin setting
    ###################################
    bytePinNo = c_byte(0)
    dwMode = c_ulong(dll.STCAM_IN_PIN_MODE_TRIGGER_INPUT)
    dll.StTrg_SetIOPinMode(camera,bytePinNo,dwMode)

    dwBufferCount = c_ulong(100)
    if not dll.StTrg_SetRawSnapShotBufferCount(camera,dwBufferCount):
        print("Couldn't set snap shot buffer.")
       
    ###################################
    # 8) Preparing image acquisition
    ###################################
    # Allocate memory for image
    imgdata = cast(create_string_buffer(bpi), POINTER(c_byte))

    # Set up display window
    cv2.namedWindow(previewName)    

    # Define the codec and create VideoWriter object.The output is stored in 'outpy.avi' file.
    # out = cv2.VideoWriter('outpy.avi',cv2.VideoWriter_fourcc('M','J','P','G'), 10, (width,height))
    if savePath != '':                
        if not os.path.exists(savePath):
            os.mkdir(savePath)
            
        prefixFileName = os.path.split(savePath)[1]
        outputFile = os.path.join(savePath,prefixFileName + "_" + previewName) + '.mp4'
        # path = r'Z:\wataru\WD_Passport\Alexei\synchrony\2019_9_12\122419'
        out = cv2.VideoWriter(outputFile, cv2.VideoWriter_fourcc('H','2','6','4'), fps, (width,height))
    
    ###################################    
    # 9) Transfer images from camera until user hits ESC
    ###################################
    # Transfer Start
    if not dll.StTrg_StartTransfer(camera):
        print("Couldn't Start.")

    cbytesxferred = c_ulong()
    cframeno = c_ulong()
    cmillisecs = c_ulong(1000)
    
    frameNum = 0
    frameTimeout = 0
    startMovie = 0
    
    while True:
        ret = dll.StTrg_TakeRawSnapShot(camera, imgdata, bpi,
                                    byref(cbytesxferred), byref(cframeno),
                                    cmillisecs)
        if not ret:
            print("Failed to transfer image from camera.", end =" ") 
            if startMovie == 1:
                frameTimeout += 1
                if frameTimeout > 2:
                    break
        else:
            startMovie = 1
            
            if  frameNum != cframeno.value - 1:
                print("Dropped a frame {}".format(cframeno.value))
            frameNum = cframeno.value

            # Make image array
            array = (c_ubyte * int(height*bpi) *
                    int(width*bpi)).from_address(addressof(imgdata.contents))

            # Convert image array to numpy so we can display it easily
            npimg = np.ndarray(buffer=array, dtype=np.uint8, shape=(height, width))

            # put the current state in the image
            im_text = "frame number: " + str(frameNum)
            add_text(npimg, im_text, 20, 0.5)

            # Write the frame into the file 'output.avi'
            if savePath != '':
                out.write(npimg)

            # Show in display window
            cv2.imshow(previewName, npimg)

        k = cv2.waitKey(1)
        if k == 27:
            # ESC to quit
            break

    ###################################
    # 10) Clean up
    ###################################
    cv2.destroyWindow(previewName)
    if savePath != '':
        out.release()

    # Free buffer
    del imgdata

    # Close the camera
    dll.StTrg_Close(camera)

###################################
# Add text in movie
def add_text(img, text, text_top, image_scale):
    """
    Args:
        img (numpy array of shape (width, height, 3): input image
        text (str): text to add to image
        text_top (int): location of top text to add
        image_scale (float): image resize scale

    Summary:
        Add display text to a frame.

    Returns:
        Next available location of top text (allows for chaining this function)
    """
    cv2.putText(
        img=img,
        text=text,
        org=(0, text_top),
        fontFace=cv2.FONT_HERSHEY_SIMPLEX,
        fontScale=image_scale,
        color=(255, 255, 255))
    return text_top + int(5 * image_scale)

"""
Sentech_camera_control.py
"""