# PCbox-for-rodent-behavior
  A simple pradigm control box (board) for rodent behavior.<BR>
  
 ![Top view of the board](PCbox.jpg)
  
  **Features**
  - Jupyter notebook interface to control Arduino inside the box.
  - Synchronize up to 3 cameras (signal level: 3.3V).
  - Auditory stimulus by a built-in soundboard using wav files.
  - TTL trigger for one shocker (Med Associate XXXX).
  - Three 28V power supplies for fans and lights.
  - Digital IOs to send triggers to your recording system.
  - Generate sinusoidal analog signal to control two external LED drivers.
  
  **Possible connections**<BR>
    [Chamber](https://www.med-associates.com/product/modular-test-chamber-with-quick-change-floor-for-rat/), 
    [Fan](https://www.med-associates.com/product/28-v-dc%e2%80%88fan/), 
    [Small light](https://www.med-associates.com/product/housestimulus-light-with-diffuser-for-rat/), 
    [Large light](https://www.med-associates.com/product/cubicle-ceiling-mount-house-light/), 
    [Shocker](https://www.med-associates.com/product/standalone-aversive-stimulatorscrambler-115-v-ac-60-hz/),
    [Speaker](https://www.med-associates.com/product/cage-speaker-for-rat-chamber/),
    [USB camera with trigger](https://sentech.co.jp/en/products/USB/index.html),
    [Recording system](https://open-ephys.org/acquisition-system/eux9baf6a5s8tid06hk1mw5aafjdz1), and
    [LED driver](https://open-ephys.org/cyclops-led-driver/cyclops-led-driver).
  
  ![connection](connection.jpg)
  
  **Caution**<BR>
  - Currently, only support Sentech USB2 camera with trigger.
  - The resolution of sinusoidal analog signal is relatively low (< 1kHz). 
  
    




