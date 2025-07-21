let current = {};
document.getElementById('start').onclick = async () => {
  const table = document.getElementById('table').value;
  // zählen
  let res = await fetch(`/api/count?table=${table}`);
  let {count} = await res.json();
  if (count==0) return alert('Keine Vokabeln mehr in dieser Richtung!');
  document.getElementById('quiz').style.display='block';
  nextWord(table);
  restartAnswerAnimation();  //Animation neu triggern
};

async function nextWord(table) {
  let res = await fetch(`/api/next?table=${table}`);
  if (res.status!=200) return alert('Durch!');
  let j = await res.json();
  current = { id: j.id, table };
  document.getElementById('frage').innerText = j.frage;
  document.getElementById('antwort').innerText = j.antwort;
}

document.getElementById('wusste').onclick = () => sendAnswer(true);
document.getElementById('oje').onclick    = () => sendAnswer(false);

// ruft die Klasse aus, für einfaches Umschalten
function restartAnswerAnimation() {
  const el = document.getElementById('antwort');
  el.classList.remove('to-black');      // Animation ausschalten
  // Reflow erzwingen, damit der Browser das Entfernen auch wirklich merkt:
  void el.offsetWidth;
  el.classList.add('to-black');         // Animation neu starten
}

async function sendAnswer(ok) {
  await fetch('/api/answer', {
    method: 'POST',
    headers: {'Content-Type':'application/json'},
    body: JSON.stringify({
      table: current.table,
      id: current.id,
      correct: ok
    })
  });
  nextWord(current.table);
  restartAnswerAnimation();  //Animation neu triggern
}