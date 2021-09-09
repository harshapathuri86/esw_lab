import './App.css';
import {BrowserRouter as Router, Route} from 'react-router-dom';
import Home from './home';
import Plotdata from './plot';
import Livedata from './live';
import '../node_modules/bootstrap/dist/css/bootstrap.min.css';

function App() {
  return (
    <Router>
      <div className="container">
        <br />
        <Route path="/" exact component={Home} />
        <Route path="/livedata" component={Livedata} />
        <Route path="/plotdata" component={Plotdata} />
      </div>
    </Router>
  );
}

export default App;
