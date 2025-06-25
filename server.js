import express from 'express';
import cors from 'cors';

const app = express();
const PORT = 3000;

app.use(cors());
app.use(express.json());

const pnrMap = {
  "345678": 1,
  "456789": 2,
  "567890": 3
};

app.post('/verify', (req, res) => {
  const { pnr, deviceId } = req.body;
  console.log(`Received PNR ${pnr} from device ${deviceId}`);

  if (!pnr || pnr.length !== 6) {
    return res.status(400).send("False PNR");
  }

  const ledIndex = pnrMap[pnr];

  if (ledIndex !== undefined) {
    console.log(`[VALID] ${pnr} → seat ${ledIndex}`);
    return res.send("PNR Exsists");
  } else {
    console.log(`[INVALID] ${pnr} not found`);
    return res.send("False PNR");
  }
});

app.listen(PORT, '0.0.0.0', () => {
  console.log(`Server running on http://0.0.0.0:${PORT}/verify`);
});
