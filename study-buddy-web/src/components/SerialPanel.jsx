import { useState } from "react";

function SerialPanel({ coins, setCoins, setWriter }) {
  const [connected, setConnected] = useState(false);
  const [port, setPort] = useState(null);
  const [reader, setReader] = useState(null);

  const listenForMessages = async (readerStream) => {
    try {
      while (true) {
        const { value, done } = await readerStream.read();

        if (done) {
          break;
        }

        if (value) {
          console.log("Received:", value);

          const message = value.trim();

        if (message.includes("SESSION_DONE")) {
        setCoins((prevCoins) => prevCoins + 1);
        console.log("Coin added!");
        }
        }
      }
    } catch (error) {
      console.error("Read error:", error);
    }
  };

  const connectToESP32 = async () => {
    try {
      const selectedPort = await navigator.serial.requestPort();
      await selectedPort.open({ baudRate: 115200 });

      // Writer setup
      const textEncoder = new TextEncoderStream();
      textEncoder.readable.pipeTo(selectedPort.writable);
      const writerStream = textEncoder.writable.getWriter();

      // Reader setup
      const textDecoder = new TextDecoderStream();
      selectedPort.readable.pipeTo(textDecoder.writable);
      const readerStream = textDecoder.readable.getReader();

      setPort(selectedPort);
      setWriter(writerStream);
      setReader(readerStream);
      setConnected(true);

      console.log("Connected to ESP32");

      listenForMessages(readerStream);
    } catch (error) {
      console.error("Connection failed", error);
    }
  };

  const sendPostureBad = async () => {
    if (!connected) return;
  };

  const sendReward = async () => {
    if (!connected) return;
  };

  const sendRecallDone = async () => {
    if (!connected) return;
  };

  return (
    <div>
      <h2>Serial Connection</h2>

      <button onClick={connectToESP32}>
        Connect to ESP32
      </button>

      <p>Status: {connected ? "Connected" : "Not Connected"}</p>
      <p>Coins: {coins}</p>
    </div>
  );
}

export default SerialPanel;