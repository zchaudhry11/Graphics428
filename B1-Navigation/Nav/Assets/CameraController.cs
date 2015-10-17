using UnityEngine;
using System.Collections;

public class CameraController : MonoBehaviour {

	private Vector3 offset;
	// Use this for initialization
	void Start () {
	}
	
	// Update is called once per frame
	void Update () {
		float moveHorizontal = Input.GetAxis ("Horizontal");
		float moveVertical = Input.GetAxis ("Vertical");
		Vector3 movement;
		
		movement = new Vector3 (moveHorizontal, 0, moveVertical);
		transform.position += movement;
	}
}
