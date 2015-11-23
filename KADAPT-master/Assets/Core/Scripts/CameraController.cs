using UnityEngine;
using System.Collections;

public class CameraController : MonoBehaviour
{
	private Vector3 offset;
	
	void Update ()
    {
		float moveHorizontal = Input.GetAxis ("Horizontal");
		float moveVertical = Input.GetAxis ("Vertical");
		Vector3 movement;
		
		movement = new Vector3 (moveHorizontal, 0, moveVertical);
		transform.position += movement;
	}
}
