import { useState } from "react";
import Home from "../components/Home";

function HomePage() {
  const [data, setData] = useState(null);

  return (
    <div>
      <h1>Home</h1>
      <Home />
    </div>
  );
}

export default HomePage;