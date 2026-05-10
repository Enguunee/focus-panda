import { useState } from "react";

function RewardPanel({ coins, setCoins, writer }) {
  const [showPopup, setShowPopup] = useState(false);
  const [result, setResult] = useState("");
  const [isSpinning, setIsSpinning] = useState(false);

  const [inventory, setInventory] = useState({
    bamboo: 0,
    apple: 0,
    fish: 0,
  });

  const loadDemoCoins = () => {
    setCoins((prev) => prev + 10);
  };

  const useItem = async (itemName) => {
    if (inventory[itemName] <= 0) return;

    setInventory((prev) => ({
      ...prev,
      [itemName]: prev[itemName] - 1,
    }));

    console.log(`${itemName} used`);

    if (writer) {
      await writer.write("REWARD_SHOW\n");
      console.log("Sent: REWARD_SHOW");
    }
  };

  const openRewardPopup = () => {
    if (coins <= 0) {
      setResult("NO COINS");
      setShowPopup(true);
      return;
    }

    setCoins((prev) => prev - 1);
    setShowPopup(true);
    setIsSpinning(true);
    setResult("");

    setTimeout(() => {
      const rewards = ["bamboo", "apple", "fish", "empty", "empty"];
      const random = rewards[Math.floor(Math.random() * rewards.length)];

      setIsSpinning(false);

      if (random === "empty") {
        setResult("NO LUCK THIS TIME");
      } else {
        setResult(`YAY! YOU WON ${random.toUpperCase()}`);

        setInventory((prev) => ({
          ...prev,
          [random]: prev[random] + 1,
        }));
      }
    }, 2000);
  };

  const closeRewardPopup = () => {
    setShowPopup(false);
  };

  return (
    <div>
      <h2>Reward Panel</h2>

      <div className="coin-box">🪙 Coins: {coins}</div>

      <button onClick={loadDemoCoins}>
        Load 10 Demo Coins
      </button>

      <br />
      <br />

      <button onClick={openRewardPopup}>
        Play
      </button>

      {showPopup && (
        <div style={overlayStyle}>
          <div style={modalStyle}>
            <h3>Reward Result</h3>

            {isSpinning ? (
              <img src="/spin.gif" alt="Spinning" style={{ width: "300px" }} />
            ) : result === "NO LUCK THIS TIME" ? (
              <>
                <img src="/angry.gif" alt="Lose" style={{ width: "300px" }} />
                <p>{result}</p>
              </>
            ) : result === "NO COINS" ? (
              <p>{result}</p>
            ) : result ? (
              <>
                <img src="/happy.gif" alt="Win" style={{ width: "300px" }} />
                <p>{result}</p>
              </>
            ) : null}

            <button onClick={closeRewardPopup}>
              Close
            </button>
          </div>
        </div>
      )}

      <div className="treasure-buttons">
        <button onClick={() => useItem("bamboo")} disabled={inventory.bamboo === 0}>
          🎋 Bamboo x{inventory.bamboo}
        </button>

        <button onClick={() => useItem("apple")} disabled={inventory.apple === 0}>
          🍎 Apple x{inventory.apple}
        </button>

        <button onClick={() => useItem("fish")} disabled={inventory.fish === 0}>
          🐟 Fish x{inventory.fish}
        </button>
      </div>
    </div>
  );
}

const overlayStyle = {
  position: "fixed",
  top: 0,
  left: 0,
  width: "100%",
  height: "100%",
  backgroundColor: "rgba(0, 0, 0, 0.5)",
  display: "flex",
  justifyContent: "center",
  alignItems: "center",
  zIndex: 1000,
};

const modalStyle = {
  backgroundColor: "white",
  padding: "40px",
  borderRadius: "20px",
  width: "700px",
  textAlign: "center",
  boxShadow: "0 12px 30px rgba(0, 0, 0, 0.2)",
};

export default RewardPanel;