import { useState } from "react";
import SerialPanel from "./components/SerialPanel";
import RewardPanel from "./components/RewardPanel";
import PosturePanel from "./components/PosturePanel";
import "./App.css";

function App() {
  const [coins, setCoins] = useState(0);
  const [writer, setWriter] = useState(null);

  return (
    <div className="app">
      <div className="header">
        <h1>🐼 Focus Panda</h1>
        <p>Focus, posture, recall, and rewards in one study companion.</p>
      </div>

      <div className="panel-grid">
        <div className="panel">
          <SerialPanel
            coins={coins}
            setCoins={setCoins}
            setWriter={setWriter}
          />
        </div>

        <div className="panel">
          <PosturePanel writer={writer} />
        </div>

        <div className="panel">
          <RewardPanel
            coins={coins}
            setCoins={setCoins}
            writer={writer}
          />
        </div>
      </div>
    </div>
  );
}

export default App;