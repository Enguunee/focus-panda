function PosturePanel({ writer }) {

    const sendBadPosture = async () => {
      if (!writer) {
        console.log("ESP32 not connected");
        return;
      }
  
      await writer.write("POSTURE_BAD\n");
      console.log("Sent: POSTURE_BAD");
    };
  
    return (
      <div>
        <h2>Posture Panel</h2>
  
        <button onClick={sendBadPosture} disabled={!writer}>
          Bad Posture Demo
        </button>
      </div>
    );
  }
  
  export default PosturePanel;