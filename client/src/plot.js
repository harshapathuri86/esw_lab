import { Component } from "react";
import { Navbar, Nav } from "react-bootstrap";

export default class Plotdata extends Component{
    constructor(props) {
        super(props);
        this.state={
      feeds:[],
        }
    }
    render() {
        return (
            <div>
                <Navbar collapseOnSelect expand="lg" bg="dark" variant="dark">
                <Navbar.Brand href="">
                  <logo alt="" width="30" height="30" className="d-inline-block align-top"/>
                </Navbar.Brand>
                <Navbar.Toggle aria-controls="responsive-navbar-nav"/>
                <Navbar.Collapse id="responsive-navbar-nav">
                  <Nav className="mr-auto">
                  <Nav.Link href="/">Home</Nav.Link>
                  <Nav.Link href="/livedata" >Live data</Nav.Link>
                  <Nav.Link href="/plotdata" className="active" >Plot data</Nav.Link>
                  </Nav>
                </Navbar.Collapse>
              </Navbar>
              <br/>
		<iframe title="temperature" width="450" height="260" style={{ border: "1px solid #cccccc"}} src="https://thingspeak.com/channels/1494903/charts/1?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&type=line&update=15"/>
<iframe title="humidity" width="450" height="260" style={{ border: "1px solid #cccccc"}} src="https://thingspeak.com/channels/1494903/charts/2?bgcolor=%23ffffff&color=%23d62020&dynamic=true&results=60&type=line&update=15"/>
		</div>
        );
    }
}