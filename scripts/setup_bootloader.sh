sudo chmod +w /etc/rc.local
sudo sed -ie "/^fi/a /home/pi/oasis-grow/bin/activate\npython3 /home/pi/oasis-growcontroller.py &" /etc/rc.local